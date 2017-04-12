#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_matrix_4x4.h"
#include "render/utils/ui_quaternion.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_transform.h"
#include "render/utils/ui_color.h"
#include "render/model/ui_data_src.h"
#include "render/cache/ui_cache_color.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "ui_runtime_render_obj_ref_i.h"
#include "ui_runtime_render_obj_child_i.h"
#include "ui_runtime_render_obj_meta_i.h"

static int ui_runtime_render_obj_ref_set_args_i(ui_runtime_render_obj_ref_t obj_ref, char * arg);

ui_runtime_render_obj_ref_t
ui_runtime_render_obj_ref_create_by_res(ui_runtime_module_t module, const char * input_res, char ** left_args) {
    char * args = NULL;
    const char * res;
    ui_runtime_render_obj_t obj;
    ui_runtime_render_obj_ref_t obj_ref;
    const char * str_value;

    if (input_res[0] == '[') {
        const char * args_end = strchr(input_res, ']');
        if (args_end == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_render_obj_ref_create: parse args: args format error!");
            return NULL;
        }

        mem_buffer_clear_data(&module->m_dump_buffer);
                              
        args = mem_buffer_strdup_len(&module->m_dump_buffer, input_res + 1, args_end - input_res - 1);

        res = args_end + 1;
    }
    else {
        res = input_res;
    }

    if ((str_value = args ? cpe_str_read_and_remove_arg(args, "name", ',', '=') : NULL)) {
        obj = ui_runtime_render_obj_find(module, str_value);
        if (obj == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_render_obj_ref_create: named obj %s not exist!", str_value);
            return NULL;
        }
    }
    else {
        UI_OBJECT_URL url_buf;
        UI_OBJECT_URL * url = ui_object_ref_parse(res, &url_buf, module->m_em);
        if (url == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_render_obj_ref_create: parse url %s fail!", input_res);
            return NULL;
        }

        obj = ui_runtime_render_obj_create_by_url(module, url, NULL);
        if (obj == NULL) return NULL;
    }

    obj_ref = ui_runtime_render_obj_ref_create_by_obj(obj);
    if (obj_ref == NULL) {
        CPE_ERROR(module->m_em, "ui_runtime_render_obj_ref_create: create obj ref fail!");

        if (obj->m_auto_release && obj->m_ref_count == 0) {
            ui_runtime_render_obj_free(obj);
        }
        
        return NULL;
    }

    if ((str_value = args ? cpe_str_read_and_remove_arg(args, "control", ',', '=') : NULL)) {
        ui_runtime_render_obj_ref_set_is_updator(obj_ref, atoi(str_value));
    }
    else if (ui_runtime_render_obj_ref_count(obj) == 1) {
        ui_runtime_render_obj_ref_set_is_updator(obj_ref, 1);
    }

    if (args && args[0]) {
        if (ui_runtime_render_obj_ref_set_args_i(obj_ref, args) != 0) {
            ui_runtime_render_obj_ref_free(obj_ref);
            return NULL;
        }
    }
    
    if (args && args[0]) ui_runtime_render_obj_setup(obj_ref->m_obj, args);

    if (left_args) {
        *left_args = (args && args[0])  ? args : NULL;
    }
    
    return obj_ref;
}

ui_runtime_render_obj_ref_t
ui_runtime_render_obj_ref_create_by_obj(ui_runtime_render_obj_t obj) {
    ui_runtime_module_t module = obj->m_module;
    ui_runtime_render_obj_ref_t obj_ref;

    obj_ref = mem_calloc(module->m_alloc, sizeof(struct ui_runtime_render_obj_ref));
    if (obj_ref == NULL) {
        CPE_ERROR(module->m_em, "ui_runtime_render_obj_ref_create: alloc obj ref fail!");
        return NULL;
    }

    obj_ref->m_module = module;
    obj_ref->m_obj = obj;
    obj_ref->m_second_color.m_mix = ui_runtime_render_second_color_none;
    obj_ref->m_second_color.m_color = UI_COLOR_WHITE;

    obj_ref->m_transform = UI_TRANSFORM_IDENTITY;
    obj_ref->m_evt_fun = NULL;
    obj_ref->m_evt_ctx = NULL;
    obj_ref->m_hide = 0;

    obj->m_ref_count++;
    TAILQ_INSERT_TAIL(&obj->m_refs, obj_ref, m_next_for_obj);

    if (ui_runtime_render_obj_ref_count(obj) == 1) {
        ui_runtime_render_obj_ref_set_is_updator(obj_ref, 1);
    }
    else {
        ui_runtime_render_obj_sync_module_update(obj);
    }

    return obj_ref;
}

ui_runtime_render_obj_ref_t
ui_runtime_render_obj_ref_create_by_obj_url(ui_runtime_module_t module, UI_OBJECT_URL const * obj_url, const char * name) {
    ui_runtime_render_obj_t obj;
    ui_runtime_render_obj_ref_t obj_ref;
    
    obj = ui_runtime_render_obj_create_by_url(module, obj_url, name);
    if (obj == NULL) return NULL;

    obj_ref = ui_runtime_render_obj_ref_create_by_obj(obj);
    if (obj_ref == NULL) {
        if (obj->m_auto_release && obj->m_ref_count == 0) {
            ui_runtime_render_obj_free(obj);
        }
    }

    if (ui_runtime_render_obj_ref_count(obj) == 1) {
        ui_runtime_render_obj_ref_set_is_updator(obj_ref, 1);
    }

    return obj_ref;
}

ui_runtime_render_obj_ref_t ui_runtime_render_obj_ref_create_by_obj_name(ui_runtime_module_t module, const char * name) {
    ui_runtime_render_obj_t obj;
    
    obj = ui_runtime_render_obj_find(module, name);
    if (obj == NULL) {
        CPE_ERROR(module->m_em, "ui_runtime_render_obj_ref_create: obj %s not exist!", name);
        return NULL;
    }

    return ui_runtime_render_obj_ref_create_by_obj(obj);
}

ui_runtime_render_obj_ref_t ui_runtime_render_obj_ref_clone(ui_runtime_render_obj_ref_t obj_ref) {
    ui_runtime_render_obj_ref_t new_obj_ref = ui_runtime_render_obj_ref_create_by_obj(obj_ref->m_obj);

    if (new_obj_ref) {
        new_obj_ref->m_hide = obj_ref->m_hide;
        new_obj_ref->m_second_color = obj_ref->m_second_color;
        new_obj_ref->m_transform = obj_ref->m_transform;
    }
    
    return new_obj_ref;
}

int ui_runtime_render_obj_ref_setup(ui_runtime_render_obj_ref_t obj_ref, char * arg_buf_will_change) {
    int rv = 0;
    const char * str_value;
    
    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control", ',', '='))) {
        ui_runtime_render_obj_ref_set_is_updator(obj_ref, atoi(str_value) ? 1 : 0);
    }
    
    if (ui_runtime_render_obj_ref_set_args_i(obj_ref, arg_buf_will_change) != 0) rv = -1;
    if (ui_runtime_render_obj_setup(obj_ref->m_obj, arg_buf_will_change) != 0) rv = -1;

    return rv;
}

void ui_runtime_render_obj_ref_disconnect(ui_runtime_render_obj_ref_t obj_ref) {
    ui_runtime_render_obj_t obj = obj_ref->m_obj;
    
    assert(obj);

    if (obj->m_updator == obj_ref) {
        ui_runtime_render_obj_ref_set_is_updator(obj_ref, 0);
        assert(obj->m_updator == NULL);
    }

    assert(obj->m_ref_count > 0);
    obj->m_ref_count--;
    TAILQ_REMOVE(&obj->m_refs, obj_ref, m_next_for_obj);

    obj_ref->m_obj = NULL;
}

void ui_runtime_render_obj_ref_free(ui_runtime_render_obj_ref_t obj_ref) {
    ui_runtime_module_t module = obj_ref->m_module;
    ui_runtime_render_obj_t obj = obj_ref->m_obj;

    if (obj) {
        ui_runtime_render_obj_ref_disconnect(obj_ref);

        if (obj->m_auto_release && obj->m_ref_count == 0) {
            ui_runtime_render_obj_free(obj);
        }
        else {
            ui_runtime_render_obj_sync_module_update(obj);
        }
    }

    mem_free(module->m_alloc, obj_ref);
}

ui_runtime_module_t ui_runtime_render_obj_ref_module(ui_runtime_render_obj_ref_t obj_ref) {
    return obj_ref->m_module;
}

ui_runtime_render_obj_t ui_runtime_render_obj_ref_obj(ui_runtime_render_obj_ref_t obj_ref) {
    return obj_ref->m_obj;
}

const char * ui_runtime_render_obj_ref_type_name(ui_runtime_render_obj_ref_t obj_ref) {
    return obj_ref->m_obj ? ui_runtime_render_obj_type_name(obj_ref->m_obj) : "";
}

void ui_runtime_render_obj_ref_transform_set_to_obj(ui_runtime_render_obj_ref_t obj_ref, ui_transform_t input_t) {
    assert(input_t);
    cpe_assert_float_sane(input_t->m_s.x);
    cpe_assert_float_sane(input_t->m_s.y);

    if (obj_ref->m_obj) {
        if (ui_transform_cmp(&obj_ref->m_transform, &UI_TRANSFORM_IDENTITY) != 0) {
            obj_ref->m_obj->m_transform = obj_ref->m_transform;
        
            if (input_t && ui_transform_cmp(input_t, &UI_TRANSFORM_IDENTITY) != 0) {
                ui_transform_adj_by_parent(&obj_ref->m_obj->m_transform, input_t);
            }
        }
        else {
            obj_ref->m_obj->m_transform = *input_t;
        }
    }
}

int ui_runtime_render_obj_ref_render(
    ui_runtime_render_obj_ref_t obj_ref, ui_runtime_render_t ctx, ui_rect_t clip_rect, ui_transform_t input_t)
{
    ui_transform transform_buf;
    ui_transform_t use_transform;
    int rv = 0;
    ui_runtime_render_obj_child_t child;
    
    if (obj_ref->m_hide) return 0;

    if (obj_ref->m_obj == NULL) return -1;
    
    if (ui_transform_cmp(&obj_ref->m_transform, &UI_TRANSFORM_IDENTITY) != 0) {
        if (input_t && ui_transform_cmp(input_t, &UI_TRANSFORM_IDENTITY) != 0) {
            transform_buf = obj_ref->m_transform;
            ui_transform_adj_by_parent(&transform_buf, input_t);
            use_transform = &transform_buf;
        }
        else {
            use_transform = &obj_ref->m_transform;
        }
    }
    else {
        use_transform = input_t;
    }

    if (obj_ref->m_obj->m_meta->m_render_fun(
            obj_ref->m_obj->m_meta->m_ctx, obj_ref->m_obj, ctx, clip_rect, &obj_ref->m_second_color, use_transform) != 0) rv = -1;

    TAILQ_FOREACH(child, &obj_ref->m_obj->m_childs, m_next_for_obj) {
        if (child->m_auto_render) {
            if (ui_runtime_render_obj_ref_render(child->m_child_obj_ref, ctx, clip_rect, use_transform) != 0) rv = -1;
        }
    }

    return rv;
}

uint8_t ui_runtime_render_obj_ref_touch_is_support(ui_runtime_render_obj_ref_t obj_ref) {
    return obj_ref->m_obj->m_touch_process_fun ? 1 : 0;
}

int ui_runtime_render_obj_ref_touch_dispatch(
    ui_runtime_render_obj_ref_t obj_ref, uint32_t track_id, ui_runtime_touch_event_t evt, ui_vector_2_t screen_pt, ui_vector_2_t input_logic_pt)
{
    struct ui_vector_2 logic_pt_buf;
    ui_vector_2_t logic_pt;

    if (obj_ref->m_obj->m_touch_process_fun == NULL) return -1;

    if (ui_transform_cmp(&obj_ref->m_transform, &UI_TRANSFORM_IDENTITY) != 0) {
        ui_transform_reverse_adj_vector_2(&obj_ref->m_transform, &logic_pt_buf, input_logic_pt);
        logic_pt = &logic_pt_buf;
    }
    else {
        logic_pt = input_logic_pt;
    }

    obj_ref->m_obj->m_touch_process_fun(obj_ref->m_obj->m_touch_process_ctx, obj_ref->m_obj, track_id, evt, screen_pt, input_logic_pt);
    return 0;
}

ui_runtime_render_second_color_t ui_runtime_render_obj_ref_second_color(ui_runtime_render_obj_ref_t obj_ref) {
	return &obj_ref->m_second_color;
}

void ui_runtime_render_obj_ref_set_second_color(ui_runtime_render_obj_ref_t obj_ref, ui_runtime_render_second_color_t second_color) {
	obj_ref->m_second_color = *second_color;
}

uint8_t ui_runtime_render_obj_ref_is_hide(ui_runtime_render_obj_ref_t obj_ref) {
    return obj_ref->m_hide;
}

void ui_runtime_render_obj_ref_set_hide(ui_runtime_render_obj_ref_t obj_ref, uint8_t hide) {
    obj_ref->m_hide = hide;
}

ui_transform_t ui_runtime_render_obj_ref_transform(ui_runtime_render_obj_ref_t obj_ref) {
    return &obj_ref->m_transform;
}

void ui_runtime_render_obj_ref_set_transform(ui_runtime_render_obj_ref_t obj_ref, ui_transform_t trans) {
    obj_ref->m_transform = *trans;
}

uint8_t ui_runtime_render_obj_ref_is_updator(ui_runtime_render_obj_ref_t obj_ref) {
    if (obj_ref->m_obj == NULL) return 0;
    return obj_ref->m_obj->m_updator == obj_ref ? 1 : 0;
}

void ui_runtime_render_obj_ref_set_is_updator(ui_runtime_render_obj_ref_t obj_ref, uint8_t is_updator) {
    ui_runtime_render_obj_t obj = obj_ref->m_obj;

    if (obj == NULL) return;
    
    if (obj->m_meta->m_update_fun == NULL) return;
    
    if (is_updator) {
        obj_ref->m_obj->m_updator = obj_ref;
    }
    else {
        if (obj->m_updator == obj_ref) {
            obj->m_updator = NULL;
        }
    }

    ui_runtime_render_obj_sync_module_update(obj);    
}

void ui_runtime_render_obj_ref_update(ui_runtime_render_obj_ref_t obj_ref, float delta) {
    ui_runtime_render_obj_t obj = obj_ref->m_obj;

    if (obj && obj->m_updator == obj_ref) {
        ui_runtime_render_obj_update(obj, delta);
    }
}

void ui_runtime_render_obj_ref_set_args(ui_runtime_render_obj_ref_t obj_ref, const char * arg) {
    char arg_buf[256];
    cpe_str_dup(arg_buf, sizeof(arg_buf), arg);
    ui_runtime_render_obj_ref_set_args_i(obj_ref, arg_buf);
}

int ui_runtime_render_obj_ref_set_args_i(ui_runtime_render_obj_ref_t obj_ref, char * args_buf) {
    ui_runtime_module_t module = obj_ref->m_module;
    int rv = 0;
    const char * p;
    ui_vector_3 s = UI_VECTOR_3_IDENTITY;
    ui_vector_3 pos = UI_VECTOR_3_ZERO;

    if ((p = cpe_str_read_and_remove_arg(args_buf, "pos-x", ',', '='))) {
        pos.x = atof(p);
    }

    if ((p = cpe_str_read_and_remove_arg(args_buf, "pos-y", ',', '='))) {
        pos.y = atof(p);
    }

    if ((p = cpe_str_read_and_remove_arg(args_buf, "pos-z", ',', '='))) {
        pos.z = atof(p);
    }
    
    if ((p = cpe_str_read_and_remove_arg(args_buf, "scale", ',', '='))) {
        s.x = s.y = s.z = atof(p);
    }
    
    if ((p = cpe_str_read_and_remove_arg(args_buf, "scale-x", ',', '='))) {
        s.x = atof(p);
    }

    if ((p = cpe_str_read_and_remove_arg(args_buf, "scale-y", ',', '='))) {
        s.y = atof(p);
    }

    if ((p = cpe_str_read_and_remove_arg(args_buf, "scale-z", ',', '='))) {
        s.z = atof(p);
    }
    
    if ((p = cpe_str_read_and_remove_arg(args_buf, "flip-x", ',', '='))) {
        s.x *= atoi(p) ? -1.0f : 0.0f;
    }

    if ((p = cpe_str_read_and_remove_arg(args_buf, "flip-y", ',', '='))) {
        s.y *= atoi(p) ? -1.0f : 0.0f;
    }

    if ((p = cpe_str_read_and_remove_arg(args_buf, "flip-z", ',', '='))) {
        s.z *= atoi(p) ? -1.0f : 0.0f;
    }

    ui_transform_set_pos_3(&obj_ref->m_transform, &pos);
    ui_transform_set_scale(&obj_ref->m_transform, &s);

    if ((p = cpe_str_read_and_remove_arg(args_buf, "angle-z", ',', '='))) {
        float angle_z = atoi(p);
        ui_quaternion r;
        ui_quaternion_set_z_radians(&r, cpe_math_angle_to_radians(angle_z));
        ui_transform_set_quation(&obj_ref->m_transform, &r);
    }

    if ((p = cpe_str_read_and_remove_arg(args_buf, "hide", ',', '='))) {
        obj_ref->m_hide = atoi(p);
    }

    if ((p = cpe_str_read_and_remove_arg(args_buf, "second-color-mix", ',', '='))) {
        obj_ref->m_second_color.m_mix = ui_runtime_render_second_color_mix_from_str(p, obj_ref->m_second_color.m_mix);
    }
    
    if ((p = cpe_str_read_and_remove_arg(args_buf, "second-color", ',', '='))) {
        ui_color c;
        if (ui_cache_find_color(module->m_cache_mgr, p, &c) != 0) {
            CPE_ERROR(module->m_em, "set obj ref attrs: second-color %s not exist!", p);
            return -1;
        }

        obj_ref->m_second_color.m_color = c;
    }

    return rv;
}

void ui_runtime_render_obj_ref_set_evt_processor(ui_runtime_render_obj_ref_t obj_ref, ui_runtime_render_obj_evt_fun_t fun, void * ctx) {
    obj_ref->m_evt_fun = fun;
    obj_ref->m_evt_ctx = ctx;
}
