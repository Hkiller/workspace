#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_rect.h"
#include "render/utils/ui_transform.h"
#include "render/utils/ui_quaternion.h"
#include "render/utils/ui_vector_3.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_action.h"
#include "render/model/ui_data_module.h"
#include "render/cache/ui_cache_res.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "render/runtime/ui_runtime_render_second_color.h"
#include "plugin_basicanim_actor_i.h"
#include "plugin_basicanim_actor_layer_i.h"
#include "plugin_basicanim_utils_i.h"

static int plugin_basicanim_actor_init(void * ctx, ui_runtime_render_obj_t render_obj) {
    struct plugin_basicanim_actor * obj = ui_runtime_render_obj_data(render_obj);
    obj->m_actor = NULL;
    obj->m_frame_time = 1.0f / 60.0f;
    obj->m_runing_time = 0.0f;
    obj->m_loop_start_frame = 0;
    obj->m_loop_count = 1;
    obj->m_is_runing = 0;
    TAILQ_INIT(&obj->m_layers);
    return 0;
}

static int plugin_basicanim_actor_set(void * ctx, ui_runtime_render_obj_t render_obj, UI_OBJECT_URL const * obj_url) {
    plugin_basicanim_module_t module = ctx;
    struct plugin_basicanim_actor * obj = ui_runtime_render_obj_data(render_obj);
    UI_OBJECT_URL_DATA_SRC_ID const * actor_id_data = &obj_url->data.actor;
    ui_data_src_t src;
    ui_data_action_t data_action;
    UI_ACTOR const * actor_data;
    struct ui_data_actor_layer_it data_layers;
    ui_data_actor_layer_t data_layer;
    
    src = ui_runtime_module_find_src(module->m_runtime, &actor_id_data->src, ui_data_src_type_action);
    if (src == NULL) return -1;

    data_action = ui_data_src_product(src);
    if (data_action == NULL) {
        CPE_ERROR(module->m_em, "src %s not loaded", ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
        return -1;
    }

    if (actor_id_data->id != (uint32_t)-1) {
        obj->m_actor = ui_data_actor_find_by_id(data_action, actor_id_data->id);
        if (obj->m_actor == NULL) {
            CPE_ERROR(module->m_em, "src %s no actor %d", ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src), actor_id_data->id);
            return -1;
        }
    }
    else if (actor_id_data->name[0]) {
        obj->m_actor = ui_data_actor_find_by_name(data_action, actor_id_data->name);
        if (obj->m_actor == NULL) {
            CPE_ERROR(module->m_em, "src %s no actor %s", ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src), actor_id_data->name);
            return -1;
        }
    }
    else {
        CPE_ERROR(module->m_em, "obj url only set src %s, no id or name", ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
        return -1;
    }
    
    actor_data = ui_data_actor_data(obj->m_actor);
    obj->m_total_frame = ui_data_actor_frame_total(obj->m_actor);
    obj->m_loop_start_frame = actor_data->loop_start;
    obj->m_loop_count = actor_data->is_loop ? 0 : 1;

    if (obj->m_loop_start_frame >= obj->m_total_frame) {
        CPE_ERROR(
            module->m_em, "src %s actor %d loop start frame %d overflow, total-frame=%d",
            ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src), actor_id_data->id,
            obj->m_loop_start_frame, obj->m_total_frame);
        goto SET_FAIL;
    }
    
    ui_data_actor_layers(&data_layers, obj->m_actor);
    while((data_layer = ui_data_actor_layer_it_next(&data_layers))) {
        plugin_basicanim_actor_layer_t layer = plugin_basicanim_actor_layer_create(module, obj, data_layer);
        if (layer == NULL) {
            CPE_ERROR(
                module->m_em, "src %s actor %d create layer fail!",
                ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src), actor_data->id);
            goto SET_FAIL;
        }
    }

    obj->m_is_runing = 1;
    ui_runtime_render_obj_set_src(render_obj, src);
    
    return 0;

SET_FAIL:
    while(!TAILQ_EMPTY(&obj->m_layers)) {
        plugin_basicanim_actor_layer_free(module, obj, TAILQ_FIRST(&obj->m_layers));
    }

    obj->m_actor = NULL;
    return -1;    
}

static void plugin_basicanim_actor_free(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_basicanim_module_t module = ctx;
    plugin_basicanim_actor_t obj = (plugin_basicanim_actor_t)ui_runtime_render_obj_data(render_obj);

    while(!TAILQ_EMPTY(&obj->m_layers)) {
        plugin_basicanim_actor_layer_free(module, obj, TAILQ_FIRST(&obj->m_layers));
    }
}

static void plugin_basicanim_actor_render_set_second_color(ui_runtime_render_second_color_t second_color, UI_TRANS const * ui_trans, ui_runtime_render_second_color_t i_second_color) {
    if (ui_trans->tex_env == 0 || ui_trans->tex_env == 3) {
        *second_color = *i_second_color;
        return;
    }
    
    second_color->m_mix = ui_trans->tex_env == 2 ? ui_runtime_render_second_color_multiply : ui_runtime_render_second_color_add;
    second_color->m_color.a = ui_trans->background.a;
    second_color->m_color.b = ui_trans->background.b;
    second_color->m_color.g = ui_trans->background.g;
    second_color->m_color.r = ui_trans->background.r;

    if(i_second_color != NULL) {
        if (i_second_color->m_mix == ui_runtime_render_second_color_add) {
            second_color->m_color.a += i_second_color->m_color.a * 255;
            second_color->m_color.r += i_second_color->m_color.r * 255;
            second_color->m_color.g += i_second_color->m_color.g * 255;
            second_color->m_color.b += i_second_color->m_color.b * 255;
            second_color->m_mix = i_second_color->m_mix;
        }            
        else if (i_second_color->m_mix == ui_runtime_render_second_color_multiply) {
            second_color->m_color.a *= i_second_color->m_color.a;
            second_color->m_color.r *= i_second_color->m_color.r;
            second_color->m_color.g *= i_second_color->m_color.g;
            second_color->m_color.b *= i_second_color->m_color.b;
            second_color->m_mix = i_second_color->m_mix;
        }         
    }
}

static void plugin_basicanim_actor_render_set_transform(ui_transform_t trans, UI_TRANS_BASE const * ui_trans) {
    ui_quaternion q = UI_QUATERNION_IDENTITY;
    ui_vector_3 s = UI_VECTOR_3_INITLIZER(ui_trans->scale.value[0], ui_trans->scale.value[1], ui_trans->scale.value[2]);
    ui_vector_3 t = UI_VECTOR_3_INITLIZER(ui_trans->trans.value[0], ui_trans->trans.value[1], ui_trans->trans.value[2]);
    ui_quaternion_set_z_radians(&q, cpe_math_angle_to_radians(ui_trans->angle));
        
    if (ui_trans->flips & 0x1) s.x *= -1.0f;
    if (ui_trans->flips & 0x2) s.y *= -1.0f;

    *trans = UI_TRANSFORM_IDENTITY;
    ui_transform_set_quation_scale(trans, &q, &s);
    ui_transform_set_pos_3(trans, &t);
}
                                                       
static int plugin_basicanim_actor_render_img_block(
    plugin_basicanim_module_t module, plugin_basicanim_actor_t obj, UI_IMG_REF const * img_ref, UI_TRANS const * img_trans,
    ui_runtime_render_t context, ui_rect_t clip_rect, ui_runtime_render_second_color_t i_second_color, ui_transform_t transform)
{
    ui_data_src_t src;
    ui_data_module_t data_module;
    ui_rect src_rect;
    ui_data_img_block_t img_block;
    ui_cache_res_t texture;
    UI_IMG_BLOCK const * img_block_data;
    struct ui_transform local_trans = UI_TRANSFORM_IDENTITY;
    struct ui_transform world_trans = UI_TRANSFORM_IDENTITY;
    struct ui_runtime_render_second_color second_color;
    ui_runtime_render_texture_filter_t filter;

    filter = img_trans->filter ==0 ? ui_runtime_render_filter_nearest : ui_runtime_render_filter_linear;
    src = ui_data_src_find_by_id(ui_runtime_module_data_mgr(module->m_runtime), img_ref->module_id);
    if (src == NULL) {
        CPE_ERROR(module->m_em, "plugin_basicanim_actor: src %d not exist", img_ref->module_id);
        return -1;
    }

    data_module = ui_data_src_product(src);
    if (data_module == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_basicanim_actor: src %s not loaded",
            ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
        return -1;
    }

    img_block = ui_data_img_block_find_by_id(data_module, img_ref->img_block_id);
    if (img_block == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_basicanim_actor: src %s no img block %d",
            ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src), img_ref->img_block_id);
        return -1;
    }
    img_block_data = ui_data_img_block_data(img_block);

    texture = ui_data_img_block_using_texture(img_block);
    if (texture == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_basicanim_actor: src %s use texture %s not exsit",
            ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src), ui_data_img_block_using_texture_path(img_block));
        return -1;
    }
    
    src_rect.lt.x = img_block_data->src_x;
    src_rect.lt.y = img_block_data->src_y;
    src_rect.rb.x = img_block_data->src_x + img_block_data->src_w;
    src_rect.rb.y = img_block_data->src_y + img_block_data->src_h;

    /*second_color*/
    plugin_basicanim_actor_render_set_second_color(&second_color, &img_ref->trans, &second_color);

    /*transform*/
    plugin_basicanim_actor_render_set_transform(&local_trans, &img_trans->local_trans);
    plugin_basicanim_actor_render_set_transform(&world_trans, &img_trans->world_trans);
    if (transform) ui_transform_adj_by_parent(&world_trans, transform);
    ui_transform_adj_by_parent(&local_trans, &world_trans);

    /*render*/
    plugin_basicanim_render_draw_rect(context, NULL, clip_rect, &local_trans, i_second_color, texture, &src_rect, filter);
    
    return 0;
}

static int plugin_basicanim_actor_render_frame(
    plugin_basicanim_module_t module, plugin_basicanim_actor_t obj, UI_FRAME_REF const * frame_ref, UI_TRANS const * frame_trans,
    ui_runtime_render_t context, ui_rect_t clip_rect, ui_runtime_render_second_color_t i_second_color, ui_transform_t transform)
{
    
    return 0;
}

static int plugin_basicanim_actor_render(
    void * ctx, ui_runtime_render_obj_t render_obj,
    ui_runtime_render_t context, ui_rect_t clip_rect, ui_runtime_render_second_color_t second_color, ui_transform_t transform)
{
    plugin_basicanim_module_t module = ctx;
    struct plugin_basicanim_actor * obj = ui_runtime_render_obj_data(render_obj);
    plugin_basicanim_actor_layer_t layer;
    int rv = 0;
    UI_TRANS trans_buf;
    UI_TRANS const * trans;
    
    TAILQ_FOREACH_REVERSE(layer, &obj->m_layers, plugin_basicanim_actor_layer_list, m_next) {
        if (layer->m_cur_frame == NULL) continue;

        /*插值 */
        trans = (layer->m_cur_frame->texture.type == UI_TEXTURE_REF_IMG
                 ? &layer->m_cur_frame->texture.data.img.trans
                 : &layer->m_cur_frame->texture.data.frame.trans);
        if (layer->m_cur_frame->smooth
            && layer->m_next_frame
            && layer->m_next_frame->start_frame > layer->m_cur_frame->start_frame)
        {
            UI_TRANS const * next_trans = (layer->m_next_frame->texture.type == UI_TEXTURE_REF_IMG
                                           ? &layer->m_next_frame->texture.data.img.trans
                                           : &layer->m_next_frame->texture.data.frame.trans);
            
            float percent = (float)(obj->m_cur_frame - layer->m_cur_frame->start_frame) / (float)(layer->m_next_frame->start_frame - layer->m_cur_frame->start_frame);

            trans_buf = *trans;
            trans_buf.local_trans.trans.value[0] += percent * (next_trans->local_trans.trans.value[0] - trans->local_trans.trans.value[0]);
            trans_buf.local_trans.trans.value[1] += percent * (next_trans->local_trans.trans.value[1] - trans->local_trans.trans.value[1]);
            trans_buf.local_trans.trans.value[2] += percent * (next_trans->local_trans.trans.value[2] - trans->local_trans.trans.value[2]);
            trans_buf.local_trans.scale.value[0] += percent * (next_trans->local_trans.scale.value[0] - trans->local_trans.scale.value[0]);
            trans_buf.local_trans.scale.value[1] += percent * (next_trans->local_trans.scale.value[1] - trans->local_trans.scale.value[1]);
            trans_buf.local_trans.scale.value[2] += percent * (next_trans->local_trans.scale.value[2] - trans->local_trans.scale.value[2]);
            trans_buf.local_trans.angle += percent * (next_trans->local_trans.angle - trans->local_trans.angle);

            trans_buf.world_trans.trans.value[0] += percent * (next_trans->world_trans.trans.value[0] - trans->world_trans.trans.value[0]);
            trans_buf.world_trans.trans.value[1] += percent * (next_trans->world_trans.trans.value[1] - trans->world_trans.trans.value[1]);
            trans_buf.world_trans.trans.value[2] += percent * (next_trans->world_trans.trans.value[2] - trans->world_trans.trans.value[2]);
            trans_buf.world_trans.scale.value[0] += percent * (next_trans->world_trans.scale.value[0] - trans->world_trans.scale.value[0]);
            trans_buf.world_trans.scale.value[1] += percent * (next_trans->world_trans.scale.value[1] - trans->world_trans.scale.value[1]);
            trans_buf.world_trans.scale.value[2] += percent * (next_trans->world_trans.scale.value[2] - trans->world_trans.scale.value[2]);
            trans_buf.world_trans.angle += percent * (next_trans->world_trans.angle - trans->world_trans.angle);

            trans_buf.background.r += percent * (next_trans->background.r - trans->background.r);
            trans_buf.background.g += percent * (next_trans->background.g - trans->background.g);
            trans_buf.background.b += percent * (next_trans->background.b - trans->background.b);
            trans_buf.background.a += percent * (next_trans->background.a - trans->background.a);
            
            trans = &trans_buf;
        }

        if(layer->m_cur_frame->texture.type == UI_TEXTURE_REF_IMG) {
            if (plugin_basicanim_actor_render_img_block(
                    module, obj, &layer->m_cur_frame->texture.data.img, trans, context, clip_rect, second_color, transform) != 0)
            {
                rv = -1;
            }
        }
        else if (layer->m_cur_frame->texture.type == UI_TEXTURE_REF_FRAME) {
            if (plugin_basicanim_actor_render_frame(
                    module, obj, &layer->m_cur_frame->texture.data.frame, trans, context, clip_rect, second_color, transform) != 0)
            {
                rv = -1;
            }
        }
    }
    
    return rv;
}

static int plugin_basicanim_actor_setup(void * ctx, ui_runtime_render_obj_t render_obj, char * args) {
    plugin_basicanim_actor_t obj = (plugin_basicanim_actor_t)ui_runtime_render_obj_data(render_obj);
    ui_data_src_t src = ui_runtime_render_obj_src(render_obj);
    plugin_basicanim_module_t module = ctx;
    char * str_value;
    int rv = 0;

    if ((str_value = cpe_str_read_and_remove_arg(args, "fps", ',', '='))) {
        float fps = atof(str_value);
        if (fps <= 0.0f) {
            CPE_ERROR(
                module->m_em, "actor obj %s(%s): fps %s error!",
                ui_runtime_render_obj_name(render_obj), src ? ui_data_src_data(src) : "", str_value);
            rv = -1;
        }
        else {
            obj->m_frame_time = 1.0f / fps;
        }
    }
    
    return rv;
}

static void plugin_basicanim_actor_update(void * ctx, ui_runtime_render_obj_t render_obj, float delta) {
    plugin_basicanim_module_t module = ctx;
    struct plugin_basicanim_actor * obj = ui_runtime_render_obj_data(render_obj);
    plugin_basicanim_actor_layer_t layer;

    if (!obj->m_is_runing) return;
    
    obj->m_runing_time += delta;
    obj->m_cur_frame = (uint16_t)(obj->m_runing_time / obj->m_frame_time);

    if(obj->m_cur_frame >= obj->m_total_frame) {
        /*播放完成 */
        TAILQ_FOREACH(layer, &obj->m_layers, m_next) {
            plugin_basicanim_actor_layer_update(module, obj, layer, obj->m_total_frame - 1);
        }

        if (obj->m_loop_count > 0) {
            obj->m_loop_count--;
            if (obj->m_loop_count == 0) {
                obj->m_is_runing = 0;
                return;
            }
        }

        TAILQ_FOREACH(layer, &obj->m_layers, m_next) {
            plugin_basicanim_actor_layer_reset(module, obj, layer);
        }

        obj->m_runing_time -= obj->m_frame_time * obj->m_total_frame;
        obj->m_cur_frame -= obj->m_total_frame - obj->m_loop_start_frame;
    }

    TAILQ_FOREACH(layer, &obj->m_layers, m_next) {
        plugin_basicanim_actor_layer_update(module, obj, layer, obj->m_cur_frame);
    }
}

static uint8_t plugin_basicanim_actor_is_playing(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_basicanim_actor_t obj = (plugin_basicanim_actor_t)ui_runtime_render_obj_data(render_obj);
    return obj->m_is_runing;
}

static void plugin_basicanim_actor_bounding(void * ctx, ui_runtime_render_obj_t render_obj, ui_rect_t bounding) {
    plugin_basicanim_actor_t obj = ui_runtime_render_obj_data(render_obj);
 
    ui_data_actor_bounding_rect(obj->m_actor, bounding);
}

int plugin_basicanim_actor_register(plugin_basicanim_module_t module) {
    ui_runtime_render_obj_meta_t obj_meta;

    obj_meta =
        ui_runtime_render_obj_meta_create(
            module->m_runtime, "actor", UI_OBJECT_TYPE_ACTOR, sizeof(struct plugin_basicanim_actor), module,
            plugin_basicanim_actor_init,
            plugin_basicanim_actor_set,
            plugin_basicanim_actor_setup,
            plugin_basicanim_actor_update,
            plugin_basicanim_actor_free,
            plugin_basicanim_actor_render,
            plugin_basicanim_actor_is_playing,
            plugin_basicanim_actor_bounding,
            NULL);
    if (obj_meta == NULL) {
        CPE_ERROR(module->m_em, "%s: create: register render obj fail", plugin_basicanim_module_name(module));
        return -1;
    }

    return 0;
}

void plugin_basicanim_actor_unregister(plugin_basicanim_module_t module) {
    ui_runtime_render_obj_meta_t obj_meta = ui_runtime_render_obj_meta_find_by_id(module->m_runtime, UI_OBJECT_TYPE_ACTOR);
    if (obj_meta) {
        ui_runtime_render_obj_meta_free(obj_meta);
    }
}
