#include <assert.h>
#include <stdio.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_json.h"
#include "render/utils/ui_transform.h"
#include "render/utils/ui_quaternion.h"
#include "render/utils/ui_color.h"
#include "render/model/ui_data_utils.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "render/runtime/ui_runtime_render_second_color.h"
#include "plugin_ui_control_frame_i.h"
#include "plugin_ui_utils_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_aspect_i.h"
#include "plugin_ui_aspect_ref_i.h"
#include "plugin_ui_animation_meta_i.h"
#include "plugin_ui_animation_i.h"
#include "plugin_ui_animation_control_i.h"

static int plugin_ui_control_frame_setup_i(plugin_ui_control_frame_t frame, char * left_args);
static void plugin_ui_control_send_page_evt(void * ctx, ui_runtime_render_obj_t obj, const char * evt);
static void plugin_ui_control_frame_update_pos_in_list(plugin_ui_control_frame_t frame);
static plugin_ui_control_frame_t plugin_ui_control_frame_find_insert_pos(plugin_ui_control_frame_t frame);

plugin_ui_control_frame_t
plugin_ui_control_frame_create(
    plugin_ui_control_t control,
    plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage,
    ui_runtime_render_obj_ref_t render_obj_ref)
{
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_control_frame_t insert_after;
    plugin_ui_control_frame_t frame;
    ui_runtime_render_obj_t render_obj = ui_runtime_render_obj_ref_obj(render_obj_ref);
    ui_rect bounding;
    
    frame = TAILQ_FIRST(&env->m_free_frames);
    if (frame) {
        TAILQ_REMOVE(&env->m_free_frames, frame, m_next);
    }
    else {
        frame = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_control_frame));
        if (frame == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_control_frame_create: alloc fail!");
            return NULL;
        }
    }

    frame->m_control = control;
    TAILQ_INIT(&frame->m_aspects);
    frame->m_layer = layer;
    frame->m_usage = usage;
    frame->m_render_obj_ref = render_obj_ref;
    frame->m_auto_remove = ui_runtime_render_obj_support_update(render_obj) ? 1 : 0;
    frame->m_priority = 0.0f;
    frame->m_sync_transform = 0;
    frame->m_sync_size = 0;
    frame->m_offset = UI_VECTOR_2_ZERO;
    frame->m_runtime_size = UI_VECTOR_2_ZERO;
    frame->m_scale = UI_VECTOR_2_IDENTITY;
    frame->m_render_size = UI_VECTOR_2_ZERO;
    frame->m_name = NULL;
    frame->m_alpha = 1.0f;
    frame->m_draw_inner = plugin_ui_control_draw_inner(control);
    
    switch(ui_runtime_render_obj_type_id(ui_runtime_render_obj_ref_obj(render_obj_ref))) {
    case UI_OBJECT_TYPE_FRAME:
    case UI_OBJECT_TYPE_IMG_BLOCK:
    case UI_OBJECT_TYPE_ACTOR:
    case UI_OBJECT_TYPE_TILEDMAP:
        frame->m_base_pos = ui_pos_policy_top_left;
        break;
    default:
        frame->m_base_pos = ui_pos_policy_center;
        break;
    }
    
    if (ui_runtime_render_obj_get_bounding(ui_runtime_render_obj_ref_obj(render_obj_ref), &bounding) != 0) {
        frame->m_offset = UI_VECTOR_2_ZERO;
        frame->m_base_size = UI_VECTOR_2_ZERO;
    }
    else {
        frame->m_base_size.x = ui_rect_width(&bounding);
        frame->m_base_size.y = ui_rect_height(&bounding);
    }

    ui_runtime_render_obj_ref_set_evt_processor(render_obj_ref, plugin_ui_control_send_page_evt, control->m_page);

    insert_after = plugin_ui_control_frame_find_insert_pos(frame);
    if (insert_after) {
        TAILQ_INSERT_AFTER(&control->m_frames, insert_after, frame, m_next);
    }
    else {
        TAILQ_INSERT_TAIL(&control->m_frames, frame, m_next);
    }

    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_frame_updated, cpe_ba_true);
    plugin_ui_control_frame_update_pos_in_list(frame);

    return frame;
}

void plugin_ui_control_frame_free(plugin_ui_control_frame_t frame) {
    plugin_ui_env_t env = frame->m_control->m_page->m_env;
    plugin_ui_control_t control = frame->m_control;

    if (frame->m_name) {
        mem_free(env->m_module->m_alloc, frame->m_name);
        frame->m_name = NULL;
    }
    
    while(!TAILQ_EMPTY(&frame->m_aspects)) {
        plugin_ui_aspect_ref_t ref = TAILQ_FIRST(&frame->m_aspects);
        plugin_ui_aspect_ref_free(ref, &ref->m_aspect->m_control_frames, &frame->m_aspects);
    }
    
    ui_runtime_render_obj_ref_free(frame->m_render_obj_ref);
    frame->m_render_obj_ref = NULL;

    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_frame_updated, cpe_ba_true);
    TAILQ_REMOVE(&control->m_frames, frame, m_next);
    
    frame->m_control = (void*)env;
    TAILQ_INSERT_TAIL(&env->m_free_frames, frame, m_next);
}

plugin_ui_control_frame_t
plugin_ui_control_frame_create_by_res(
    plugin_ui_control_t control,
    plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, const char * res)
{
    plugin_ui_env_t env;
    plugin_ui_control_frame_t frame;
    ui_runtime_render_obj_ref_t obj_ref;
    char * left_args;
    
    assert(control->m_page);
    assert(&control->m_page->m_control.m_is_free);

    env = control->m_page->m_env;

    obj_ref = ui_runtime_render_obj_ref_create_by_res(env->m_module->m_runtime, res, &left_args);
    if (obj_ref == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "control %s: load %s obj_ref from res %s fail!",
            plugin_ui_control_path_dump(&env->m_module->m_dump_buffer, control),
            plugin_ui_control_frame_layer_name(layer),
            res);
        return NULL;
    }

    frame = plugin_ui_control_frame_create(control, layer, usage, obj_ref);
    if (frame == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "control %s: load %s frame from res %s fail!",
            plugin_ui_control_path_dump(&env->m_module->m_dump_buffer, control),
            plugin_ui_control_frame_layer_name(layer),
            res);
        ui_runtime_render_obj_ref_free(obj_ref);
        return NULL;
    }
    
    /*resize相关设置 */
    if (left_args) {
        if (plugin_ui_control_frame_setup_i(frame, left_args) != 0) {
            plugin_ui_control_frame_free(frame);
            return NULL;
        }
    }
    
    return frame;
}

plugin_ui_control_frame_t
plugin_ui_control_frame_create_by_type(
    plugin_ui_control_t control, plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, const char * type, const char * args)
{
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_control_frame_t frame;
    ui_runtime_render_obj_t obj;
    ui_runtime_render_obj_ref_t obj_ref;
    char * tmp_args = NULL;
    
    if (args) {
        mem_buffer_clear_data(&env->m_module->m_dump_buffer);
        tmp_args = mem_buffer_strdup(&env->m_module->m_dump_buffer, args);
    }
    
    obj = ui_runtime_render_obj_create_by_type(env->m_module->m_runtime, NULL, type);
    if (obj == NULL) return NULL;
    
    obj_ref = ui_runtime_render_obj_ref_create_by_obj(obj);
    if (obj_ref == NULL) {
        ui_runtime_render_obj_free(obj);
        return NULL;
    }

    if (tmp_args && ui_runtime_render_obj_ref_setup(obj_ref, tmp_args) != 0) {
        ui_runtime_render_obj_ref_free(obj_ref);
        return NULL;
    }
    
    frame = plugin_ui_control_frame_create(control, layer, usage, obj_ref);
    if (frame == NULL) {
        ui_runtime_render_obj_ref_free(obj_ref);
        return NULL;
    }

    if (tmp_args && plugin_ui_control_frame_setup_i(frame, tmp_args) != 0) {
        ui_runtime_render_obj_ref_free(obj_ref);
        return NULL;
    }
    
    return frame;
}

plugin_ui_control_frame_t
plugin_ui_control_frame_create_by_def(
    plugin_ui_control_t control,
    plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, 
    UI_CONTROL_RES_REF const * res_ref)
{
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_control_frame_t frame;
    ui_runtime_render_obj_t obj;
    ui_runtime_render_obj_ref_t obj_ref;
    struct ui_runtime_render_second_color second_color;
    UI_OBJECT_URL url;

    if (res_ref->res.type == UI_OBJECT_TYPE_NONE) return NULL;

    if (ui_data_control_ref_to_object_ref(&url, &res_ref->res) != 0
        || (obj = ui_runtime_render_obj_create_by_url(env->m_module->m_runtime, &url, NULL)) == NULL)
    {
        struct mem_buffer buff;
        mem_buffer_init(&buff, env->m_module->m_alloc);
        CPE_ERROR(
            env->m_module->m_em, "control %s: load %s frame from url %s %s fail!",
            plugin_ui_control_path_dump(&env->m_module->m_dump_buffer, control),
            plugin_ui_control_frame_layer_name(layer),
			plugin_ui_control_name(control),
            dr_json_dump_inline(&buff, res_ref, sizeof(*res_ref), ui_data_mgr_meta_control_object_url(env->m_module->m_data_mgr)));
        mem_buffer_clear(&buff);
        return NULL;
    }
    
    obj_ref = ui_runtime_render_obj_ref_create_by_obj(obj);
    if (obj_ref == NULL) {
        ui_runtime_render_obj_free(obj);
        return NULL;
    }

    frame = plugin_ui_control_frame_create(control, layer, usage, obj_ref);
    if (frame == NULL) {
        ui_runtime_render_obj_ref_free(obj_ref);
        return NULL;
    }

    second_color.m_color.a = res_ref->color.a;
    second_color.m_color.r = res_ref->color.r;
    second_color.m_color.g = res_ref->color.g;
    second_color.m_color.b = res_ref->color.b;
    second_color.m_mix = ui_runtime_render_second_color_none;

    ui_runtime_render_obj_ref_set_second_color(frame->m_render_obj_ref, &second_color);

    if (cpe_ba_get(&control->m_flag, plugin_ui_control_flag_disable_global_resize)) {
        frame->m_offset.x = res_ref->erect.lt;
        frame->m_offset.y = res_ref->erect.tp;
        frame->m_runtime_size.x = res_ref->erect.rt - res_ref->erect.lt;
        frame->m_runtime_size.y = res_ref->erect.bm - res_ref->erect.tp;
    }
    else {
        frame->m_offset.x = res_ref->erect.lt * env->m_screen_adj.x;
        frame->m_offset.y = res_ref->erect.tp * env->m_screen_adj.y;
        frame->m_runtime_size.x = (res_ref->erect.rt - res_ref->erect.lt * (1.0f - env->m_screen_adj.x)) - frame->m_offset.x;
        frame->m_runtime_size.y = (res_ref->erect.bm - res_ref->erect.tp * (1.0f - env->m_screen_adj.y)) - frame->m_offset.y;
    }

    return frame;
}

plugin_ui_control_frame_t
plugin_ui_control_frame_create_by_frame(
    plugin_ui_control_t control, plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, plugin_ui_control_frame_t from_frame)
{
    ui_runtime_render_obj_ref_t obj_ref;
    plugin_ui_control_frame_t frame;

    obj_ref = ui_runtime_render_obj_ref_create_by_obj(plugin_ui_control_frame_render_obj(from_frame));
    if (obj_ref == NULL) {
        CPE_ERROR(control->m_page->m_env->m_module->m_em, "plugin_ui_control_frame_create_by_frame: create obj ref fail!");
        return NULL;
    }
        
    frame = plugin_ui_control_frame_create(control, layer, usage, obj_ref);
    if (frame == NULL) {
        CPE_ERROR(control->m_page->m_env->m_module->m_em, "plugin_ui_control_frame_create_by_frame: create frame fail!");
        ui_runtime_render_obj_ref_free(obj_ref);
        return NULL;
    }

    frame->m_auto_remove = from_frame->m_auto_remove;
    frame->m_base_pos = from_frame->m_base_pos;
    frame->m_sync_transform = from_frame->m_sync_transform;
    frame->m_sync_size = from_frame->m_sync_size;
    frame->m_base_size = from_frame->m_base_size;
    frame->m_priority = from_frame->m_priority;
    frame->m_offset = from_frame->m_offset;
    frame->m_scale = from_frame->m_scale;
    frame->m_runtime_size = from_frame->m_runtime_size;
    
    return frame;
}

plugin_ui_control_t plugin_ui_control_frame_control(plugin_ui_control_frame_t frame) {
    return frame->m_control;
}

const char * plugin_ui_control_frame_name(plugin_ui_control_frame_t frame) {
    return frame->m_name ? frame->m_name : "";
}

void plugin_ui_control_frame_set_layer(plugin_ui_control_frame_t frame, plugin_ui_control_frame_layer_t layer) {
    frame->m_layer = layer;
    plugin_ui_control_frame_update_pos_in_list(frame);
}

plugin_ui_control_frame_usage_t plugin_ui_control_frame_usage(plugin_ui_control_frame_t frame) {
    return frame->m_usage;
}

void plugin_ui_control_frame_set_usage(plugin_ui_control_frame_t frame, plugin_ui_control_frame_usage_t usage) {
    frame->m_usage = usage;
}

int plugin_ui_control_frame_set_name(plugin_ui_control_frame_t frame, const char * name) {
    plugin_ui_module_t module = frame->m_control->m_page->m_env->m_module;

    if (frame->m_name) {
        mem_free(module->m_alloc, frame->m_name);
    }

    if (name) {
        frame->m_name = cpe_str_mem_dup_trim(module->m_alloc, name);
        if (frame->m_name == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_control_frame_set_name: dup fail!");
            return -1;
        }
    }
    else {
        frame->m_name = NULL;
    }

    return 0;
}

plugin_ui_control_frame_t
plugin_ui_control_frame_find_by_name(plugin_ui_control_t control, const char * name) {
    plugin_ui_control_frame_t frame;
    
    TAILQ_FOREACH(frame, &control->m_frames, m_next) {
        if (frame->m_name && strcmp(frame->m_name, name) == 0) return frame;
    }

    return NULL;
}

plugin_ui_control_frame_t
plugin_ui_control_frame_find_by_local_pt(plugin_ui_control_t control, ui_vector_2_t pt) {
    plugin_ui_control_frame_t frame;
    
    TAILQ_FOREACH(frame, &control->m_frames, m_next) {
        if (frame->m_render_size.x > 0.0f && frame->m_render_size.y > 0.0f) {
            if (pt->x > frame->m_offset.x
                && (pt->x - frame->m_offset.x) < frame->m_render_size.x
                && pt->y > frame->m_offset.y
                && (pt->y - frame->m_offset.y) < frame->m_render_size.y)
            {
                return frame;
            }
                
        }
    }

    return NULL;
}

plugin_ui_control_frame_t
plugin_ui_control_frame_find_by_render_obj_type(plugin_ui_control_t control, const char * name) {
    plugin_ui_control_frame_t frame;
    
    TAILQ_FOREACH(frame, &control->m_frames, m_next) {
        if (strcmp(ui_runtime_render_obj_type_name(ui_runtime_render_obj_ref_obj(frame->m_render_obj_ref)), name) == 0) return frame;
    }

    return NULL;
}

int plugin_ui_control_frame_setup(plugin_ui_control_frame_t frame, char * arg_buf_will_change) {
    int rv = 0;

    if (ui_runtime_render_obj_ref_setup(frame->m_render_obj_ref, arg_buf_will_change) != 0) rv = -1;
    if (plugin_ui_control_frame_setup_i(frame, arg_buf_will_change) != 0) rv = -1;

    return rv;
}

ui_vector_2_t plugin_ui_control_frame_runtime_size(plugin_ui_control_frame_t frame) {
    return &frame->m_runtime_size;
}

void plugin_ui_control_frame_set_runtime_size(plugin_ui_control_frame_t frame, ui_vector_2_t size) {
    frame->m_runtime_size = *size;
}

ui_vector_2_t plugin_ui_control_frame_scale(plugin_ui_control_frame_t frame) {
    return &frame->m_scale;
}

void plugin_ui_control_frame_set_scale(plugin_ui_control_frame_t frame, ui_vector_2_t scale) {
    frame->m_scale.x = scale->x;
    frame->m_scale.y = scale->y;
}

void plugin_ui_control_do_render_frame(
    plugin_ui_control_frame_t frame, ui_color_t color, ui_rect_t render_rect, ui_runtime_render_t ctx, ui_rect_t clip_rect)
{
    plugin_ui_control_t control = frame->m_control;
    plugin_ui_env_t env = control->m_page->m_env;
    ui_vector_2 render_sz;
    ui_transform o = UI_TRANSFORM_IDENTITY;
    ui_vector_3 o_t;
    ui_vector_3 o_s;
    ui_vector_2 policy_trans;
    ui_color real_color;
    struct ui_runtime_render_second_color save_second_color;
    struct ui_runtime_render_second_color second_color;
    uint8_t need_second_color;

    cpe_assert_float_sane(render_rect->lt.x);
    cpe_assert_float_sane(render_rect->lt.y);
    cpe_assert_float_sane(render_rect->rb.x);
    cpe_assert_float_sane(render_rect->rb.y);

    real_color = control->m_render_color;
    if(color) ui_color_inline_mul(&real_color, color);
    real_color.a *= frame->m_alpha;
    
    if (real_color.a == 0.0f) return;
    
    need_second_color = ui_color_cmp(&real_color, &UI_COLOR_WHITE) == 0 ? 0 : 1;

    o_s.x = control->m_all_frame_scale.x * frame->m_scale.x;
    o_s.y = control->m_all_frame_scale.y * frame->m_scale.y;
    o_s.z = 1.0f;

    if (cpe_float_cmp(o_s.x, 0.0f, UI_FLOAT_PRECISION) == 0 || cpe_float_cmp(o_s.x, 0.0f, UI_FLOAT_PRECISION) == 0) {
        return;
    }
    
    render_sz.x = frame->m_runtime_size.x > 0.0f ? frame->m_runtime_size.x * control->m_render_scale.x : ui_rect_width(render_rect);
    render_sz.y = frame->m_runtime_size.y > 0.0f ? frame->m_runtime_size.y * control->m_render_scale.y : ui_rect_height(render_rect);
    if (render_sz.x <= 0.0f || render_sz.y <= 0.0f) return;
    
    if (frame->m_runtime_size.x > 0.0f && frame->m_runtime_size.y > 0.0f) {
        o_s.x *= control->m_render_scale.x;
        o_s.y *= control->m_render_scale.y;
        
        /*原始尺寸所放到binding */
        if (frame->m_base_size.x > 0.0f) o_s.x *= frame->m_runtime_size.x / frame->m_base_size.x;
        if (frame->m_base_size.y > 0.0f) o_s.y *= frame->m_runtime_size.y / frame->m_base_size.y;

        /*binding等比缩放为控件大小 */
        /* if (control->m_editor_sz.x > 0.0f) o_s.x *= render_sz.x / control->m_editor_sz.x; */
        /* if (control->m_editor_sz.y > 0.0f) o_s.y *= render_sz.y / control->m_editor_sz.y; */
        ui_assert_vector_2_sane(&control->m_render_sz_ns);
        if (control->m_editor_sz.x > 0.0f) o_s.x *= control->m_render_sz_ns.x / control->m_editor_sz.x;
        if (control->m_editor_sz.y > 0.0f) o_s.y *= control->m_render_sz_ns.y / control->m_editor_sz.y;
    }
    else {
        if (frame->m_base_size.x > 0.0f) {
            o_s.x *= render_sz.x / frame->m_base_size.x;

            if (frame->m_base_size.y > 0.0f) {
                o_s.y *= render_sz.y / frame->m_base_size.y;
            }
            else {
                o_s.y *= render_sz.x / frame->m_base_size.x;
            }
        }
        else {
            if (frame->m_base_size.y > 0.0f) {
                o_s.x *= render_sz.y / frame->m_base_size.y;
                o_s.y *= render_sz.y / frame->m_base_size.y;
            }
            else {
                if (!cpe_ba_get(&control->m_flag, plugin_ui_control_flag_disable_global_resize)) {
                    o_s.x *= env->m_screen_adj.x;
                    o_s.y *= env->m_screen_adj.y;
                }

                o_s.x *= control->m_render_scale.x;
                o_s.y *= control->m_render_scale.y;
            }
        }
    }

    if (cpe_float_cmp(control->m_render_angle.z, 0.0f, UI_FLOAT_PRECISION) != 0) {
        ui_quaternion o_r;
        ui_quaternion_set_z_radians(&o_r, cpe_math_angle_to_radians(control->m_render_angle.z));
        ui_transform_set_quation_scale(&o, &o_r, &o_s);
    }
    else {
        ui_transform_set_scale(&o, &o_s);
    }
    
    policy_trans = plugin_ui_calc_adj_sz_by_pos_policy(&render_sz, &control->m_pivot, frame->m_base_pos);

    o_t.x =
        render_rect->lt.x /*绘制的基础位置 */
        + render_sz.x * 0.5f * (1.0f - control->m_all_frame_scale.x) /*缩放引起的位置偏移 */
        + policy_trans.x /*基准点位置 */
        + (control->m_all_frame_pt.x /*控件设定的位置偏移 */
           + frame->m_offset.x) /*frame设定的附加偏移 */ * control->m_render_scale.x;
    
    o_t.y = render_rect->lt.y
        + render_sz.y * 0.5f * (1.0f - control->m_all_frame_scale.x)
        + policy_trans.y
        + (control->m_all_frame_pt.y
           + frame->m_offset.y) * control->m_render_scale.y;
    
    o_t.z = 0.0f;
    
    /*带有pivot的旋转 */
    if (ui_vector_2_cmp(&control->m_pivot, &UI_VECTOR_2_ZERO) != 0
        && cpe_float_cmp(control->m_render_angle.z, 0.0f, UI_FLOAT_PRECISION) != 0)
    {
        ui_vector_2 adj;
        ui_vector_2 base_pos;
        float distance;
        float angle_rad;

        assert(0);
        adj.x = control->m_render_sz_ns.x * o_s.x * control->m_pivot.x;
        adj.y = control->m_render_sz_ns.y * o_s.y * control->m_pivot.y;

        if (!cpe_ba_get(&control->m_flag, plugin_ui_control_flag_disable_global_resize)) {
            adj.x /= control->m_page->m_env->m_screen_adj.x;
            adj.y /= control->m_page->m_env->m_screen_adj.y;
        }
        
        base_pos.x = render_rect->lt.x + adj.x;
        base_pos.y = render_rect->lt.y + adj.y;
        
        distance = cpe_math_distance(base_pos.x, base_pos.y, o_t.x, o_t.y);
        angle_rad = cpe_math_radians(base_pos.x, base_pos.y, o_t.x, o_t.y);
        
        angle_rad += cpe_math_angle_to_radians(control->m_render_angle.z);
        
        o_t.x = base_pos.x + distance * cpe_cos_radians(angle_rad);
        o_t.y = base_pos.y + distance * cpe_sin_radians(angle_rad);
    }

    ui_transform_set_pos_3(&o, &o_t);
    
    if (need_second_color) {
        save_second_color = second_color = *ui_runtime_render_obj_ref_second_color(frame->m_render_obj_ref);

        if (second_color.m_mix == ui_runtime_render_second_color_none) {
            second_color.m_color = real_color;
            second_color.m_mix = ui_runtime_render_second_color_multiply;
        }
        else if (second_color.m_mix == ui_runtime_render_second_color_add) {
            ui_color_inline_add(&second_color.m_color, &real_color);
        }            
        else if (second_color.m_mix == ui_runtime_render_second_color_multiply) {
            ui_color_inline_mul(&second_color.m_color, &real_color);
        }            
            
        ui_runtime_render_obj_ref_set_second_color(frame->m_render_obj_ref, &second_color);
    }

    if (frame->m_sync_transform) {
        ui_runtime_render_obj_ref_transform_set_to_obj(frame->m_render_obj_ref, &o);
    }

    if (frame->m_sync_size) {
        ui_vector_2 sz;
        sz.x = ui_rect_width(render_rect) / env->m_screen_adj.x / control->m_render_scale.x;
        sz.y = ui_rect_height(render_rect) / env->m_screen_adj.y / control->m_render_scale.y;
        ui_runtime_render_obj_set_size(ui_runtime_render_obj_ref_obj(frame->m_render_obj_ref), &sz);
    }
        
    ui_runtime_render_obj_ref_render(frame->m_render_obj_ref, ctx, clip_rect, &o);

    if (need_second_color) {
        ui_runtime_render_obj_ref_set_second_color(frame->m_render_obj_ref, &save_second_color);
    }
}

void plugin_ui_control_frame_clear(plugin_ui_control_t control, plugin_ui_aspect_t asspect) {
    plugin_ui_control_frame_t frame, next;

    for(frame = TAILQ_FIRST(&control->m_frames); frame; frame = next) {
        next = TAILQ_NEXT(frame, m_next);
        if (asspect == NULL || plugin_ui_aspect_control_frame_is_in(asspect, frame)) {
            plugin_ui_control_frame_free(frame);
        }
    }
}

void plugin_ui_control_frame_clear_in_layer(plugin_ui_control_t control, plugin_ui_control_frame_layer_t layer, plugin_ui_aspect_t aspect) {
    plugin_ui_control_frame_t frame, next;

    for(frame = TAILQ_FIRST(&control->m_frames); frame; frame = next) {
        next = TAILQ_NEXT(frame, m_next);
        if (frame->m_layer == layer && (aspect == NULL || plugin_ui_aspect_control_frame_is_in(aspect, frame))) {
            plugin_ui_control_frame_free(frame);
        }
    }
}

void plugin_ui_control_frame_clear_by_usage(plugin_ui_control_t control, plugin_ui_control_frame_usage_t usage, plugin_ui_aspect_t aspect) {
    plugin_ui_control_frame_t frame, next;

    for(frame = TAILQ_FIRST(&control->m_frames); frame; frame = next) {
        next = TAILQ_NEXT(frame, m_next);
        if (frame->m_usage == usage && (aspect == NULL || plugin_ui_aspect_control_frame_is_in(aspect, frame))) {
            plugin_ui_control_frame_free(frame);
        }
    }
}

void plugin_ui_control_frame_clear_by_layer_and_usage(
    plugin_ui_control_t control,
    plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage, plugin_ui_aspect_t aspect)
{
    plugin_ui_control_frame_t frame, next;

    for(frame = TAILQ_FIRST(&control->m_frames); frame; frame = next) {
        next = TAILQ_NEXT(frame, m_next);
        if (frame->m_usage == usage && frame->m_layer == layer && (aspect == NULL || plugin_ui_aspect_control_frame_is_in(aspect, frame))) {
            plugin_ui_control_frame_free(frame);
        }
    }
}

uint32_t plugin_ui_control_frame_count_by_usage(plugin_ui_control_t control, plugin_ui_control_frame_usage_t usage) {
    plugin_ui_control_frame_t frame;
    uint32_t count = 0;
    
    TAILQ_FOREACH(frame, &control->m_frames, m_next) {
        if (frame->m_usage == usage) count++;
    }

    return count;
}

plugin_ui_control_frame_layer_t plugin_ui_control_frame_layer(plugin_ui_control_frame_t frame) {
    return frame->m_layer;
}

const char * plugin_ui_control_frame_layer_str(plugin_ui_control_frame_t frame) {
    return plugin_ui_control_frame_layer_to_str(frame->m_layer);
}

float plugin_ui_control_frame_alpha(plugin_ui_control_frame_t frame) {
    return frame->m_alpha;
}

void plugin_ui_control_frame_set_alpha(plugin_ui_control_frame_t frame, float alpha) {
    frame->m_alpha = alpha;
}

uint8_t plugin_ui_control_frame_visible(plugin_ui_control_frame_t frame) {
    return ui_runtime_render_obj_ref_is_hide(frame->m_render_obj_ref) ? 0 : 1;
}

void plugin_ui_control_frame_set_visible(plugin_ui_control_frame_t frame, uint8_t visible) {
    ui_runtime_render_obj_ref_set_hide(frame->m_render_obj_ref, visible ? 0 : 1);
}

ui_runtime_render_obj_t plugin_ui_control_frame_render_obj(plugin_ui_control_frame_t frame) {
    return ui_runtime_render_obj_ref_obj(frame->m_render_obj_ref);
}

void * plugin_ui_control_frame_render_obj_data(plugin_ui_control_frame_t frame) {
    return ui_runtime_render_obj_data(ui_runtime_render_obj_ref_obj(frame->m_render_obj_ref));
}

ui_runtime_render_obj_ref_t plugin_ui_control_frame_render_obj_ref(plugin_ui_control_frame_t frame) {
    return frame->m_render_obj_ref;
}

float plugin_ui_control_frame_priority(plugin_ui_control_frame_t frame) {
    return frame->m_priority;
}

void plugin_ui_control_frame_set_priority(plugin_ui_control_frame_t frame, float priority) {
    frame->m_priority = priority;
    plugin_ui_control_frame_update_pos_in_list(frame);
}

uint8_t plugin_ui_control_frame_base_pos(plugin_ui_control_frame_t frame) {
    return frame->m_base_pos;
}

void plugin_ui_control_frame_set_base_pos(plugin_ui_control_frame_t frame, uint8_t base_pos) {
    frame->m_base_pos = base_pos;
}

ui_vector_2_t plugin_ui_control_frame_base_size(plugin_ui_control_frame_t frame) {
    return &frame->m_base_size;
}

void plugin_ui_control_frame_set_base_size(plugin_ui_control_frame_t frame, ui_vector_2_t base_size) {
    frame->m_base_size = *base_size;
}

uint8_t plugin_ui_control_frame_auto_remove(plugin_ui_control_frame_t frame) {
    return frame->m_auto_remove;
}

void plugin_ui_control_frame_set_auto_remove(plugin_ui_control_frame_t frame, uint8_t auto_remove) {
    frame->m_auto_remove = auto_remove;
}

void plugin_ui_control_frame_real_free(plugin_ui_control_frame_t frame) {
    plugin_ui_env_t env = (void*)frame->m_control;

    TAILQ_REMOVE(&env->m_free_frames, frame, m_next);

    mem_free(env->m_module->m_alloc,  frame);
}

static void plugin_ui_control_frame_read_base_size_x(plugin_ui_control_frame_t frame, char * args) {
    const char * str_value;
    char value_buf[64];

    if ((str_value = cpe_str_read_and_remove_arg(args, "ui-base-size.x", ',', '='))) {
        frame->m_base_size.x = atof(str_value);
    }
    else if (cpe_str_read_arg(value_buf, sizeof(value_buf), args, "ui-base-size", ',', '=') == 0) {
        frame->m_base_size.x = atof(value_buf);
    }
    else {
        ui_rect bounding;
        if (ui_runtime_render_obj_get_bounding(ui_runtime_render_obj_ref_obj(frame->m_render_obj_ref), &bounding) == 0) {
            frame->m_base_size.x = ui_rect_width(&bounding);
        }
        else {
            UI_CONTROL const * self_data =
                frame->m_control->m_src ? ui_data_control_data(frame->m_control->m_src)
                : frame->m_control->m_template ? ui_data_control_data(frame->m_control->m_template)
                : NULL;
            if (self_data) {
                frame->m_base_size.x = self_data->basic.editor_sz.value[0];
            }
        }
    }
}

static void plugin_ui_control_frame_read_base_size_y(plugin_ui_control_frame_t frame, char * args) {
    const char * str_value;
    char value_buf[64];
    
    if ((str_value = cpe_str_read_and_remove_arg(args, "ui-base-size.y", ',', '='))) {
        frame->m_base_size.y = atof(value_buf);
    }
    else if (cpe_str_read_arg(value_buf, sizeof(value_buf), args, "ui-base-size", ',', '=') == 0) {
        frame->m_base_size.y = atof(value_buf);
    }
    else {
        ui_rect bounding;
        if (ui_runtime_render_obj_get_bounding(ui_runtime_render_obj_ref_obj(frame->m_render_obj_ref), &bounding) == 0) {
            frame->m_base_size.y = ui_rect_height(&bounding);
        }
        else {
            UI_CONTROL const * self_data =
                frame->m_control->m_src ? ui_data_control_data(frame->m_control->m_src)
                : frame->m_control->m_template ? ui_data_control_data(frame->m_control->m_template)
                : NULL;
            if (self_data) {
                frame->m_base_size.y = self_data->basic.editor_sz.value[1];
            }
        }
    }
}

static void plugin_ui_control_send_page_evt(void * ctx, ui_runtime_render_obj_t obj, const char * evt) {
    plugin_ui_page_build_and_send_event(ctx, evt, NULL);
}

static int plugin_ui_control_frame_setup_anim(plugin_ui_control_frame_t frame, const char * anim_type, char * left_args) {
    plugin_ui_env_t env =frame->m_control->m_page->m_env;
    plugin_ui_animation_meta_t animation_meta;
    plugin_ui_animation_t animation;

    animation_meta = plugin_ui_animation_meta_find(env->m_module, anim_type);
    if (animation_meta == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "control %s: frame setup: ui animation %s not exist",
            plugin_ui_control_path_dump(&env->m_module->m_dump_buffer, frame->m_control), anim_type);
        return -1;
    }

    animation = plugin_ui_animation_create(env, animation_meta);
    if (animation == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "control %s: frame setup: ui animation %s create fail",
            plugin_ui_control_path_dump(&env->m_module->m_dump_buffer, frame->m_control), anim_type);
        return -1;
    }

    if (plugin_ui_animation_setup(animation, left_args, frame->m_control, frame) != 0) {
        CPE_ERROR(
            env->m_module->m_em, "control %s: frame setup: ui animation %s setup fail",
            plugin_ui_control_path_dump(&env->m_module->m_dump_buffer, frame->m_control), anim_type);
        plugin_ui_animation_free(animation);
        return -1;
    }

    return 0;
}

static int plugin_ui_control_frame_setup_i(plugin_ui_control_frame_t frame, char * left_args) {
    int rv = 0;
    const char * str_value;

    if ((str_value = cpe_str_read_and_remove_arg(left_args, "ui-resize", ',', '='))) {
        frame->m_base_size = UI_VECTOR_2_ZERO;
        if (atoi(str_value)) {
            plugin_ui_control_frame_read_base_size_x(frame, left_args);
            plugin_ui_control_frame_read_base_size_y(frame, left_args);
        }
        else {
            frame->m_base_size = UI_VECTOR_2_ZERO;
        }
    }
    else if ((str_value = cpe_str_read_and_remove_arg(left_args, "ui-resize-x", ',', '='))) {
        frame->m_base_size.x = 0.0F;
        if (atoi(str_value)) {
            plugin_ui_control_frame_read_base_size_x(frame, left_args);
        }
    }
    else if ((str_value = cpe_str_read_and_remove_arg(left_args, "ui-resize-y", ',', '='))) {
        frame->m_base_size.y = 0.0F;
        if (atoi(str_value)) {
            plugin_ui_control_frame_read_base_size_y(frame, left_args);
        }
    }

    if ((str_value = cpe_str_read_and_remove_arg(left_args, "ui-base-pos", ',', '='))) {
        frame->m_base_pos = ui_pos_policy_from_str(str_value);
    }

    if ((str_value = cpe_str_read_and_remove_arg(left_args, "ui-auto-remove", ',', '='))) {
        frame->m_auto_remove = atoi(str_value) ? 1 : 0;
    }

    if ((str_value = cpe_str_read_and_remove_arg(left_args, "ui-frame-name", ',', '='))) {
        plugin_ui_control_frame_set_name(frame, str_value);
    }
    
    if ((str_value = cpe_str_read_and_remove_arg(left_args, "ui-adj-pos.x", ',', '='))) {
        frame->m_offset.x = atof(str_value);
    }

    if ((str_value = cpe_str_read_and_remove_arg(left_args, "ui-adj-pos.y", ',', '='))) {
        frame->m_offset.y = atof(str_value);
    }

    if ((str_value = cpe_str_read_and_remove_arg(left_args, "ui-priority", ',', '='))) {
        frame->m_priority = atof(str_value);
        plugin_ui_control_frame_update_pos_in_list(frame);
    }

    if ((str_value = cpe_str_read_and_remove_arg(left_args, "sync-transform", ',', '='))) {
        frame->m_sync_transform = atoi(str_value);
    }

    if ((str_value = cpe_str_read_and_remove_arg(left_args, "sync-size", ',', '='))) {
        frame->m_sync_size = atoi(str_value);
        if (frame->m_sync_size) {
            ui_vector_2 sz;
            ui_rect render_rect;

            if (plugin_ui_control_need_update_cache(frame->m_control)) {
                plugin_ui_control_check_update_from_root(frame->m_control);
            }

            if (frame->m_draw_inner) {
                render_rect = plugin_ui_control_real_inner_abs(frame->m_control);
            }
            else {
                render_rect = plugin_ui_control_real_rt_abs(frame->m_control);
            }

            sz.x = ui_rect_width(&render_rect) / frame->m_control->m_page->m_env->m_screen_adj.x / frame->m_control->m_render_scale.x;
            sz.y = ui_rect_height(&render_rect) / frame->m_control->m_page->m_env->m_screen_adj.y / frame->m_control->m_render_scale.y;

            ui_runtime_render_obj_set_size(ui_runtime_render_obj_ref_obj(frame->m_render_obj_ref), &sz);
        }
    }
    
    if ((str_value = cpe_str_read_and_remove_arg(left_args, "ui-world-pos", ',', '='))) {
        ui_vector_2 world_pos;
        if (plugin_ui_env_calc_world_pos(&world_pos, frame->m_control->m_page->m_env, str_value) != 0) {
            CPE_ERROR(
                frame->m_control->m_page->m_env->m_module->m_em,
                "plugin_ui_control_frame_setup: get pos from %s fail!", str_value);
            rv = -1;
        }
        else {
            plugin_ui_control_frame_set_world_pos(frame, &world_pos);
        }
    }
    
    while ((str_value = cpe_str_read_and_remove_arg(left_args, "ui-anim", ',', '='))) {
        if (plugin_ui_control_frame_setup_anim(frame, str_value, left_args) != 0) {
            rv = -1;
        }
    }

    return rv;
}

static void plugin_ui_control_frame_update_pos_in_list(plugin_ui_control_frame_t frame) {
    plugin_ui_control_frame_t insert_after = plugin_ui_control_frame_find_insert_pos(frame);

    TAILQ_REMOVE(&frame->m_control->m_frames, frame, m_next);
    
    if (insert_after) {
        TAILQ_INSERT_AFTER(&frame->m_control->m_frames, insert_after, frame, m_next);
    }
    else {
        TAILQ_INSERT_HEAD(&frame->m_control->m_frames, frame, m_next);
    }
}

struct plugin_ui_control_frame_it_data {
    plugin_ui_control_frame_t m_frame;
    uint8_t m_layer;
    uint8_t m_usage;
};

static void plugin_ui_control_frame_check_to_next(struct plugin_ui_control_frame_it_data * data) {
    while(data->m_frame) {
        if (data->m_layer != (uint8_t)-1) {
            if (data->m_frame->m_layer != (plugin_ui_control_frame_layer_t)data->m_layer) {
                data->m_frame = TAILQ_NEXT(data->m_frame, m_next);
                continue;
            }
        }

        if (data->m_usage != (uint8_t)-1) {
            if (data->m_frame->m_usage != (plugin_ui_control_frame_usage_t)data->m_usage) {
                data->m_frame = TAILQ_NEXT(data->m_frame, m_next);
                continue;
            }
        }
        
        break;
    }
}

static plugin_ui_control_frame_t plugin_ui_control_frame_next(struct plugin_ui_control_frame_it * it) {
    struct plugin_ui_control_frame_it_data * data = (struct plugin_ui_control_frame_it_data *)it->m_data;
    plugin_ui_control_frame_t r;

    if (data->m_frame == NULL) return NULL;

    r = data->m_frame;

    data->m_frame = TAILQ_NEXT(r, m_next);
    
    plugin_ui_control_frame_check_to_next(data);
    
    return r;
}

void plugin_ui_control_frames(plugin_ui_control_t control, plugin_ui_control_frame_it_t frame_it) {
    struct plugin_ui_control_frame_it_data * data = (struct plugin_ui_control_frame_it_data *)frame_it->m_data;
    data->m_frame = TAILQ_FIRST(&control->m_frames);
    data->m_layer = (uint8_t)-1;
    data->m_usage = (uint8_t)-1;
    frame_it->next = plugin_ui_control_frame_next;
}

void plugin_ui_control_frames_in_layer(plugin_ui_control_t control, plugin_ui_control_frame_it_t frame_it, plugin_ui_control_frame_layer_t layer) {
    struct plugin_ui_control_frame_it_data * data = (struct plugin_ui_control_frame_it_data *)frame_it->m_data;
    data->m_frame = TAILQ_FIRST(&control->m_frames);
    data->m_layer = (uint8_t)layer;
    data->m_usage = (uint8_t)-1;
    plugin_ui_control_frame_check_to_next(data);
    frame_it->next = plugin_ui_control_frame_next;
}

void plugin_ui_control_frames_by_usage(plugin_ui_control_t control, plugin_ui_control_frame_it_t frame_it, plugin_ui_control_frame_usage_t usage) {
    struct plugin_ui_control_frame_it_data * data = (struct plugin_ui_control_frame_it_data *)frame_it->m_data;
    data->m_frame = TAILQ_FIRST(&control->m_frames);
    data->m_layer = (uint8_t)-1;
    data->m_usage = (uint8_t)usage;
    plugin_ui_control_frame_check_to_next(data);
    frame_it->next = plugin_ui_control_frame_next;
}

void plugin_ui_control_frames_by_layer_and_usage(
    plugin_ui_control_t control, plugin_ui_control_frame_it_t frame_it,
    plugin_ui_control_frame_layer_t layer, plugin_ui_control_frame_usage_t usage)
{
    struct plugin_ui_control_frame_it_data * data = (struct plugin_ui_control_frame_it_data *)frame_it->m_data;
    data->m_frame = TAILQ_FIRST(&control->m_frames);
    data->m_layer = (uint8_t)layer;
    data->m_usage = (uint8_t)usage;
    plugin_ui_control_frame_check_to_next(data);
    frame_it->next = plugin_ui_control_frame_next;
}

void plugin_ui_control_frame_print(write_stream_t s, plugin_ui_control_frame_t frame) {
    ui_runtime_render_obj_t render_obj = ui_runtime_render_obj_ref_obj(frame->m_render_obj_ref);
    ui_data_src_t src = ui_runtime_render_obj_src(render_obj);
    const char * obj_name = ui_runtime_render_obj_name(render_obj);
    
    stream_printf(
        s, "%s.%s.%s[",
        plugin_ui_page_name(frame->m_control->m_page),
        plugin_ui_control_name(frame->m_control),
        plugin_ui_control_frame_layer_name(frame->m_layer));

    if (obj_name && obj_name[0]) {
        stream_printf(s, "%s|", obj_name);
    }

    if (src) {
        ui_data_src_path_print(s, src);
    }
    else {
        stream_printf(s, "%s", ui_runtime_render_obj_type_name(render_obj));
    }
    
    stream_putc(s, ']');
}

const char * plugin_ui_control_frame_dump(mem_buffer_t buff, plugin_ui_control_frame_t frame) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buff);

    mem_buffer_clear_data(buff);

    plugin_ui_control_frame_print((write_stream_t)&stream, frame);

    stream_putc((write_stream_t)&stream, 0);

    return (const char *)mem_buffer_make_continuous(buff, 0);
}

const char * plugin_ui_control_frame_layer_name(plugin_ui_control_frame_layer_t layer) {
    switch(layer) {
    case plugin_ui_control_frame_layer_back:
        return "back";
    case plugin_ui_control_frame_layer_tail:
        return "tail";
    case plugin_ui_control_frame_layer_text:
        return "text";
    default:
        return "unknown-frame-laery";
    }
}

plugin_ui_control_frame_t plugin_ui_control_frame_find_touchable(plugin_ui_control_t control) {
    plugin_ui_control_frame_t frame;

    TAILQ_FOREACH(frame, &control->m_frames, m_next) {
        if (ui_runtime_render_obj_ref_touch_is_support(frame->m_render_obj_ref)) {
            return frame;
        }
    }
    
    return NULL;
}

uint8_t plugin_ui_control_frame_touch_is_support(plugin_ui_control_frame_t frame) {
    return ui_runtime_render_obj_ref_touch_is_support(frame->m_render_obj_ref);
}

int plugin_ui_control_frame_touch_dispatch(plugin_ui_control_frame_t frame, uint32_t track_id, ui_runtime_touch_event_t evt, ui_vector_2_t pt) {
    plugin_ui_env_t env = frame->m_control->m_page->m_env;
    ui_vector_2 frame_pt;
    
    if (!frame->m_sync_transform) {
        CPE_ERROR(
            env->m_module->m_em, "control %s: frame touch dispatch: frame not sync transform, can`t dispatch touch event",
            plugin_ui_control_path_dump(&env->m_module->m_dump_buffer, frame->m_control));
        return -1;
    }

    frame_pt.x = pt->x / env->m_screen_adj.x;
    frame_pt.y = pt->y / env->m_screen_adj.y;

    /* CPE_ERROR(env->m_module->m_em, "xxxxx: frame_touch_dispatch: pt=(%f,%f), screen_adj=(%f,%f), frame_pt=(%f,%f)\n", */
    /*        pt->x, pt->y, env->m_screen_adj.x, env->m_screen_adj.y, frame_pt.x, frame_pt.y); */
    
    ui_runtime_render_obj_ref_touch_dispatch(frame->m_render_obj_ref, track_id, evt, pt, &frame_pt);
    return 0;
}

ui_vector_2_t plugin_ui_control_frame_local_pos(plugin_ui_control_frame_t frame) {
    return &frame->m_offset;
}

void plugin_ui_control_frame_set_local_pos(plugin_ui_control_frame_t frame, ui_vector_2_t local_pos) {
    frame->m_offset = *local_pos;
}

ui_vector_2 plugin_ui_control_frame_world_pos(plugin_ui_control_frame_t frame) {
    struct ui_vector_2 world_pos;
    ui_vector_2_t control_pt = plugin_ui_control_real_pt_abs(frame->m_control);
    ui_vector_2_t control_scale = plugin_ui_control_scale(frame->m_control);
    world_pos.x = control_pt->x + frame->m_offset.x * control_scale->x;
    world_pos.y = control_pt->y + frame->m_offset.y * control_scale->y;
    return world_pos;
}

void plugin_ui_control_frame_set_world_pos(plugin_ui_control_frame_t frame, ui_vector_2_t world_pos) {
    ui_vector_2_t control_pt = plugin_ui_control_real_pt_abs(frame->m_control);
    ui_vector_2_t control_scale = plugin_ui_control_scale(frame->m_control);
    frame->m_offset.x = (world_pos->x - control_pt->x) / control_scale->x;
    frame->m_offset.y = (world_pos->y - control_pt->y) / control_scale->y;
}

ui_rect plugin_ui_control_frame_local_rect(plugin_ui_control_frame_t frame) {
    ui_rect r;
    ui_vector_2_t control_sz = plugin_ui_control_real_sz_no_scale(frame->m_control);
    r.lt = frame->m_offset;
    r.rb.x = frame->m_render_size.x > 0.0f ? (r.lt.x + frame->m_render_size.x) : control_sz->x;
    r.rb.y = frame->m_render_size.y > 0.0f ? (r.lt.y + frame->m_render_size.y) : control_sz->y;
    return r;
}

void plugin_ui_control_frame_set_local_rect(plugin_ui_control_frame_t frame, ui_rect_t rect) {
    plugin_ui_control_frame_set_local_pos(frame, &rect->lt);
    frame->m_render_size.x = ui_rect_width(rect);
    frame->m_render_size.y = ui_rect_height(rect);
}

ui_vector_2_t plugin_ui_control_frame_render_size(plugin_ui_control_frame_t frame) {
    return &frame->m_render_size;
}

void plugin_ui_control_frame_set_render_size(plugin_ui_control_frame_t frame, ui_vector_2_t size) {
    frame->m_render_size = *size;
}

uint8_t plugin_ui_control_frame_sync_size(plugin_ui_control_frame_t frame) {
    return frame->m_sync_size;
}

void plugin_ui_control_frame_set_sync_size(plugin_ui_control_frame_t frame, uint8_t sync_size) {
    sync_size = sync_size ? 1 : 0;
    frame->m_sync_size = sync_size;
}

const char * plugin_ui_control_frame_obj_type_name(plugin_ui_control_frame_t frame) {
    return ui_runtime_render_obj_ref_type_name(frame->m_render_obj_ref);
}

void plugin_ui_control_frame_cancel_animations(plugin_ui_control_frame_t frame, plugin_ui_aspect_t aspect) {
    plugin_ui_animation_control_t animation_control, next;
    
    for(animation_control = TAILQ_FIRST(&frame->m_control->m_animations); animation_control; animation_control = next) {
        plugin_ui_aspect_t anim_aspect;
        
        next = TAILQ_NEXT(animation_control, m_next_for_control);

        if (aspect && !plugin_ui_aspect_animation_is_in(aspect, animation_control->m_animation)) continue;

        anim_aspect = plugin_ui_animation_aspect(animation_control->m_animation);
        if (anim_aspect == NULL || !plugin_ui_aspect_control_frame_is_in(anim_aspect, frame)) continue;
        
        if (animation_control->m_is_tie) {
            plugin_ui_animation_free(animation_control->m_animation);
        }
        else {
            plugin_ui_animation_control_free(animation_control);
        }
    }
}

plugin_ui_animation_t plugin_ui_control_frame_create_animation(plugin_ui_control_frame_t frame, char * str_value) {
    plugin_ui_env_t env = frame->m_control->m_page->m_env;
    const char * anim_type;
    char * anim_args;
    char * sep;
    plugin_ui_animation_meta_t animation_meta;
    plugin_ui_animation_t animation;

    sep = strchr(str_value, ':');
    if (sep == NULL) {
        * cpe_str_trim_tail(str_value + strlen(str_value), str_value) = 0;
        anim_type = str_value;
        anim_args = NULL;
    }
    else {
        * cpe_str_trim_tail(sep, str_value) = 0;
        anim_type = str_value;
        anim_args = cpe_str_trim_head(sep + 1);
    }

    animation_meta = plugin_ui_animation_meta_find(env->m_module, anim_type);
    if (animation_meta == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_control_frame_create_animation: animation type %s not exist", anim_type);
        return NULL;
    }

    animation = plugin_ui_animation_create(env, animation_meta);
    if (animation == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_control_frame_create_animation: animation type %s create fail!", anim_type);
        return NULL;
    }

    if (anim_args) {
        if (plugin_ui_animation_setup(animation, anim_args, frame->m_control, frame) != 0) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_control_frame_create_animation: animation type %s setup fail!", anim_type);
            plugin_ui_animation_free(animation);
            return NULL;
        }
    }

    return animation;
}

const char * plugin_ui_control_frame_layer_to_str(plugin_ui_control_frame_layer_t layer) {
    switch(layer) {
    case plugin_ui_control_frame_layer_back:
        return "back";
    case plugin_ui_control_frame_layer_text:
        return "text";
    case plugin_ui_control_frame_layer_tail:
        return "tail";
    default:
        return "unknown";
    }
}

int plugin_ui_control_frame_str_to_layer(const char * str_layer, plugin_ui_control_frame_layer_t * r) {
    if (strcmp(str_layer, "back") == 0) {
        *r = plugin_ui_control_frame_layer_back;
        return 0;
    }
    else if (strcmp(str_layer, "text") == 0) {
        *r = plugin_ui_control_frame_layer_text;
        return 0;
    }
    else if (strcmp(str_layer, "tail") == 0) {
        *r = plugin_ui_control_frame_layer_tail;
        return 0;
    }
    else {
        return -1;
    }
}

static plugin_ui_control_frame_t plugin_ui_control_frame_find_insert_pos(plugin_ui_control_frame_t frame) {
    plugin_ui_control_t control = frame->m_control;
    plugin_ui_control_frame_t check;
    
    TAILQ_FOREACH_REVERSE(check, &control->m_frames, plugin_ui_control_frame_list, m_next) {
        if (check == frame) continue;
        if (check->m_layer > frame->m_layer) continue;
        if (check->m_layer < frame->m_layer) return check;
        if (check->m_priority > frame->m_priority) continue;
        return check;
    }

    return NULL;
}
