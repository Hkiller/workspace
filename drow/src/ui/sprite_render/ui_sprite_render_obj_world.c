#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "render/utils/ui_rect.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_transform.h"
#include "render/utils/ui_quaternion.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui_sprite_render_obj_world_i.h"
#include "ui_sprite_render_env_i.h"
#include "ui_sprite_render_env_touch_processor_i.h"
#include "ui_sprite_render_layer_i.h"
#include "ui_sprite_render_anim_i.h"

int ui_sprite_render_obj_world_init(void * ctx, ui_runtime_render_obj_t render_obj) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_obj_world_t obj = ui_runtime_render_obj_data(render_obj);
    obj->m_module = ctx;
    obj->m_control_tick = 0;
    obj->m_sync_transform = 0;
    obj->m_env = NULL;

    ui_runtime_render_obj_touch_set_processor(render_obj, ui_sprite_render_env_touch_process, module);
    
    return 0;
}

void ui_sprite_render_obj_world_free(void * ctx, ui_runtime_render_obj_t render_obj) {
    ui_sprite_render_obj_world_t obj = ui_runtime_render_obj_data(render_obj);
    ui_sprite_world_t world;

    if (obj->m_control_tick && obj->m_env) {
        world = ui_sprite_world_res_world(ui_sprite_world_res_from_data(obj->m_env));
        ui_sprite_world_start_tick(world);
    }

    if (obj->m_env) {
        TAILQ_REMOVE(&obj->m_env->m_renders, obj, m_next_for_env);
    }
}

ui_sprite_render_env_t ui_sprite_render_obj_world_env(ui_sprite_render_obj_world_t world_obj) {
    return world_obj->m_env;
}

void ui_sprite_render_obj_world_set_env(ui_sprite_render_obj_world_t world_obj, ui_sprite_render_env_t env) {
    if (world_obj->m_env == env) return;
    
    if (world_obj->m_env) {
        TAILQ_REMOVE(&world_obj->m_env->m_renders, world_obj, m_next_for_env);
    }

    world_obj->m_env = env;

    if (world_obj->m_env) {
        ui_sprite_world_t world = ui_sprite_world_res_world(ui_sprite_world_res_from_data(world_obj->m_env));
        
        TAILQ_INSERT_TAIL(&world_obj->m_env->m_renders, world_obj, m_next_for_env);

        if (world_obj->m_control_tick) {
            ui_sprite_world_stop_tick(world);
        }
        else {
            ui_sprite_world_start_tick(world);
        }            
    }
}

int ui_sprite_render_obj_world_render(
    void * ctx, ui_runtime_render_obj_t render_obj,
    ui_runtime_render_t context, ui_rect_t clip_rect, ui_runtime_render_second_color_t second_color, ui_transform_t transform)
{
    ui_sprite_render_obj_world_t obj = ui_runtime_render_obj_data(render_obj);
    ui_sprite_render_env_t env;
    ui_sprite_render_layer_t layer;
    ui_transform camera_o;

    if (obj->m_env == NULL) return -1;
    env = obj->m_env;
    
    camera_o = env->m_transform;
    ui_transform_adj_by_parent(&camera_o, transform);

    TAILQ_FOREACH_REVERSE(layer, &env->m_layers, ui_sprite_render_layer_list, m_next) {
        ui_sprite_render_anim_t anim;
        
        if (layer->m_is_dirty) {
            ui_sprite_render_layer_sort_anims(layer);
            layer->m_is_dirty = 0;
        }

        TAILQ_FOREACH(anim, &layer->m_anims, m_next_for_layer) {
            ui_transform anim_trans = anim->m_transform;
            ui_transform_adj_by_parent(&anim_trans, &camera_o);
            ui_runtime_render_obj_ref_render(anim->m_render_obj_ref, context, clip_rect, &anim_trans);
        }
    }

    return 0;
}

static void ui_sprite_render_obj_world_update(void * ctx, ui_runtime_render_obj_t render_obj, float delta) {
    ui_sprite_render_obj_world_t obj = ui_runtime_render_obj_data(render_obj);
    
    if (obj->m_env == NULL) return;

    if (obj->m_sync_transform) {
        obj->m_env->m_base_transform = *ui_runtime_render_obj_transform(render_obj);
    }
    
    if (obj->m_control_tick) {
        ui_sprite_world_t world = ui_sprite_world_res_world(ui_sprite_world_res_from_data(obj->m_env));
        ui_sprite_world_tick(world, delta);
    }
}

static uint8_t ui_sprite_render_obj_world_is_playing(void * ctx, ui_runtime_render_obj_t render_obj) {
    ui_sprite_render_obj_world_t obj = ui_runtime_render_obj_data(render_obj);
    return obj->m_env ? 1 : 0;
}

static void ui_sprite_render_obj_world_bounding(void * ctx, ui_runtime_render_obj_t render_obj, ui_rect_t bounding) {
    ui_sprite_render_obj_world_t obj = ui_runtime_render_obj_data(render_obj);
    
    *bounding = UI_RECT_ZERO;

    if (obj->m_env) {
        bounding->rb = obj->m_env->m_design_size;
    }
}

static int ui_sprite_render_obj_world_resize(void * ctx, ui_runtime_render_obj_t render_obj, ui_vector_2_t size) {
    ui_sprite_render_obj_world_t obj = ui_runtime_render_obj_data(render_obj);
    ui_sprite_render_env_t render_env;
    struct ui_transform trans = UI_TRANSFORM_IDENTITY;
    ui_vector_3 s;

    render_env = obj->m_env;
    if (render_env == NULL) return -1;

    if (render_env->m_design_size.x > 0.0f) {
        if (render_env->m_design_size.y == 0.0f) {
            s.x = size->x / render_env->m_design_size.x;
            s.y = s.x;
            s.z = 1.0f;

            render_env->m_size.x = render_env->m_design_size.x;
            render_env->m_size.y = size->y / s.x;
        }
        else {
            s.x = size->x / render_env->m_design_size.x;
            s.y = size->y / render_env->m_design_size.y;
            s.z = 1.0f;

            render_env->m_size.x = render_env->m_design_size.x;
            render_env->m_size.y = render_env->m_design_size.y;
        }
    }
    else {
        if (render_env->m_design_size.y > 0.0f) {
            s.y = size->y / render_env->m_design_size.y;
            s.x = s.y;
            s.z = 1.0f;

            render_env->m_size.y = render_env->m_design_size.y;
            render_env->m_size.x = size->x / s.y;

            //printf("xxxxx: scale-x=%f, scale-y=%f, size=(%f,%f)\n", s.x, s.y, size->x, size->y);
        }
        else {
            s.y = 1.0f;
            s.x = 1.0f;
            s.z = 1.0f;
            render_env->m_size = *size;
        }
    }

    ui_transform_set_scale(&trans, &s);
    ui_sprite_render_env_set_base_transform(render_env, &trans);

    return 0;
}

static int ui_sprite_render_obj_world_setup(void * ctx, ui_runtime_render_obj_t render_obj, char * arg_buf_will_change) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_obj_world_t obj = ui_runtime_render_obj_data(render_obj);
    const char * str_value;
    char value_buf[32];

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "world", ',', '='))) {
        ui_sprite_world_t world;
        ui_sprite_render_env_t env;

        world = ui_sprite_world_find(module->m_repo, str_value);
        if (world == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_render_obj_world_setup: world %s not exist!", str_value);
            return -1;
        }

        env = ui_sprite_render_env_find(world);
        if (env == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_render_obj_world_setup: world %s no render env!", str_value);
            return -1;
        }

        ui_sprite_render_obj_world_set_env(obj, env);
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "control-tick", ',', '='))) {
        ui_sprite_render_obj_world_set_control_tick(obj, atoi(str_value));
    }

    if (cpe_str_read_arg(value_buf, sizeof(value_buf), arg_buf_will_change, "sync-transform", ',', '=') == 0) {
        ui_sprite_render_obj_world_set_sync_transform(obj, atoi(value_buf));
    }
    
    return 0;
}

uint8_t ui_sprite_render_obj_world_control_tick(ui_sprite_render_obj_world_t world_obj) {
    return world_obj->m_control_tick;
}

void ui_sprite_render_obj_world_set_control_tick(ui_sprite_render_obj_world_t world_obj, uint8_t control_tick) {
    if (control_tick) control_tick = 1;

    if (world_obj->m_control_tick == control_tick) return;

    world_obj->m_control_tick = control_tick;

    if (world_obj->m_env) {
        ui_sprite_world_t world = ui_sprite_world_res_world(ui_sprite_world_res_from_data(world_obj->m_env));
        if (control_tick) {
            ui_sprite_world_stop_tick(world);
        }
        else {
            ui_sprite_world_start_tick(world);
        }
    }
}

uint8_t ui_sprite_render_obj_world_sync_transform(ui_sprite_render_obj_world_t world_obj) {
    return world_obj->m_sync_transform;
}

void ui_sprite_render_obj_world_set_sync_transform(ui_sprite_render_obj_world_t world_obj, uint8_t sync_transform) {
    if (sync_transform) sync_transform = 1;

    if (world_obj->m_sync_transform == sync_transform) return;

    world_obj->m_sync_transform = sync_transform;
}

ui_sprite_world_t ui_sprite_render_obj_world_world(ui_sprite_render_obj_world_t world_obj) {
    if (world_obj->m_env == NULL) return NULL;
    return ui_sprite_world_res_world(ui_sprite_world_res_from_data(world_obj->m_env));
}

int ui_sprite_render_obj_world_regist(ui_sprite_render_module_t module) {
    ui_runtime_render_obj_meta_t obj_meta;
        
    if (module->m_runtime == NULL) return 0;
    
    obj_meta =
        ui_runtime_render_obj_meta_create(
            module->m_runtime, "world", 0, sizeof(struct ui_sprite_render_obj_world), module,
            ui_sprite_render_obj_world_init,
            NULL,
            ui_sprite_render_obj_world_setup,
            ui_sprite_render_obj_world_update,
            ui_sprite_render_obj_world_free,
            ui_sprite_render_obj_world_render,
            ui_sprite_render_obj_world_is_playing,
            ui_sprite_render_obj_world_bounding,
            ui_sprite_render_obj_world_resize);
    if (obj_meta == NULL) {
        CPE_ERROR(module->m_em, "%s: create: register render obj world fail", ui_sprite_render_module_name(module));
        return -1;
    }

    return 0;
}

void ui_sprite_render_obj_world_unregist(ui_sprite_render_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta = ui_runtime_render_obj_meta_find_by_name(module->m_runtime, "world");
        if (obj_meta) {
            ui_runtime_render_obj_meta_free(obj_meta);
        }
    }
}
