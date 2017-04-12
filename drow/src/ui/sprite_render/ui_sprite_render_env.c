#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "gd/app/app_context.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui_sprite_render_env_i.h"
#include "ui_sprite_render_env_touch_processor_i.h"
#include "ui_sprite_render_env_transform_monitor_i.h"
#include "ui_sprite_render_layer_i.h"
#include "ui_sprite_render_anim_i.h"
#include "ui_sprite_render_obj_world_i.h"

static void ui_sprite_render_env_update(ui_sprite_world_t world, void * ctx, float delta_s);

static void ui_sprite_render_env_clear(ui_sprite_world_res_t world_res, void * ctx) {
    ui_sprite_world_t world = ui_sprite_world_res_world(world_res);
    ui_sprite_render_env_t env = (ui_sprite_render_env_t)ui_sprite_world_res_data(world_res);

    ui_sprite_world_remove_updator(world, env);

    while(!TAILQ_EMPTY(&env->m_layers)) {
        ui_sprite_render_layer_free(TAILQ_FIRST(&env->m_layers));
    }
    assert(env->m_default_layer == NULL);

    assert(cpe_hash_table_count(&env->m_anims) == 0);
    cpe_hash_table_fini(&env->m_anims);

    while(!TAILQ_EMPTY(&env->m_renders)) {
        ui_sprite_render_obj_world_set_env(TAILQ_FIRST(&env->m_renders), NULL);
    }
    
    while(!TAILQ_EMPTY(&env->m_free_anims)) {
        ui_sprite_render_anim_real_free(TAILQ_FIRST(&env->m_free_anims));
    }

    while(!TAILQ_EMPTY(&env->m_touch_processors)) {
        ui_sprite_render_env_touch_processor_free(TAILQ_FIRST(&env->m_touch_processors));
    }

    while(!TAILQ_EMPTY(&env->m_transform_monitors)) {
        ui_sprite_render_env_transform_monitor_free(TAILQ_FIRST(&env->m_transform_monitors));
    }
}

ui_sprite_render_env_t
ui_sprite_render_env_create(ui_sprite_render_module_t module, ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res;
    ui_sprite_render_env_t env;

    world_res =
        ui_sprite_world_res_create(
            world, UI_SPRITE_RENDER_ENV_NAME, sizeof(struct ui_sprite_render_env));

    if (world_res == NULL) {
        CPE_ERROR(module->m_em, "create render env: creat res fail!");
        return NULL;
    }

    env = (ui_sprite_render_env_t)ui_sprite_world_res_data(world_res);

    bzero(env, sizeof(*env));

    env->m_module = module;
    env->m_default_layer = NULL;
    TAILQ_INIT(&env->m_layers);
    env->m_max_id = 0;

    env->m_base_transform = UI_TRANSFORM_IDENTITY;
    env->m_transform = UI_TRANSFORM_IDENTITY;

    TAILQ_INIT(&env->m_renders);
    TAILQ_INIT(&env->m_global_anims);
    TAILQ_INIT(&env->m_free_anims);
    TAILQ_INIT(&env->m_touch_processors);
    TAILQ_INIT(&env->m_transform_monitors);

    if (ui_sprite_world_add_updator(world, ui_sprite_render_env_update, env) != 0) {
        CPE_ERROR(module->m_em, "create render env: add updator fail!");
        ui_sprite_world_res_free(world_res);
        return NULL;
    }

    if (cpe_hash_table_init(
            &env->m_anims,
            module->m_alloc,
            (cpe_hash_fun_t) ui_sprite_render_anim_hash,
            (cpe_hash_eq_t) ui_sprite_render_anim_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_render_anim, m_hh),
            -1) != 0)
    {
        CPE_ERROR(module->m_em, "create render env: init anims hash table fail!");
        ui_sprite_world_res_free(world_res);
        return NULL;
    }
    
    env->m_default_layer = ui_sprite_render_layer_create(env, NULL, "default");
    if (env->m_default_layer == NULL) {
        assert(TAILQ_EMPTY(&env->m_layers));
        cpe_hash_table_fini(&env->m_anims);
        ui_sprite_world_res_free(world_res);
        return NULL;
    }
    
    ui_sprite_world_res_set_free_fun(world_res, ui_sprite_render_env_clear, NULL);

    return env;
}

void ui_sprite_render_env_free(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_RENDER_ENV_NAME);
    if (world_res) {
        ui_sprite_world_res_free(world_res);
    }
}

ui_sprite_render_env_t ui_sprite_render_env_find(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_RENDER_ENV_NAME);
    return world_res ? (ui_sprite_render_env_t)ui_sprite_world_res_data(world_res) : NULL;
}

int ui_sprite_render_env_set_update_priority(ui_sprite_render_env_t env, int8_t priority) {
    ui_sprite_world_res_t res = ui_sprite_world_res_from_data(env);
    ui_sprite_world_t world = ui_sprite_world_res_world(res);
    return ui_sprite_world_set_updator_priority(world, env, priority);
}

void ui_sprite_render_env_set_debug(ui_sprite_render_env_t env, uint8_t debug) {
    env->m_debug = debug;
}

ui_vector_2_t ui_sprite_render_env_design_size(ui_sprite_render_env_t env) {
    return &env->m_design_size;
}

void ui_sprite_render_env_set_design_size(ui_sprite_render_env_t env, ui_vector_2_t sz) {
    env->m_design_size = *sz;
}

ui_vector_2_t ui_sprite_render_env_size(ui_sprite_render_env_t env) {
    return &env->m_size;
}

void ui_sprite_render_env_set_size(ui_sprite_render_env_t env, ui_vector_2_t sz) {
    env->m_size = *sz;
}

ui_transform_t ui_sprite_render_env_base_transform(ui_sprite_render_env_t env) {
    return &env->m_base_transform;
}

void ui_sprite_render_env_set_base_transform(ui_sprite_render_env_t env, ui_transform_t transform) {
    env->m_base_transform = *transform;
}

ui_transform_t ui_sprite_render_env_transform(ui_sprite_render_env_t env) {
    return &env->m_transform;
}

void ui_sprite_render_env_set_transform(ui_sprite_render_env_t env, ui_transform_t transform) {
    ui_sprite_render_env_transform_monitor_t processor;
    
    env->m_transform = *transform;
    
    TAILQ_FOREACH(processor, &env->m_transform_monitors, m_next) {
        processor->m_process_fun(processor->m_process_ctx, env);
    }
}

ui_vector_2 ui_sprite_render_env_screen_to_world(ui_sprite_render_env_t env, ui_vector_2_t i_pos) {
    ui_vector_2 pos = *i_pos;
    ui_transform_inline_reverse_adj_vector_2(&env->m_base_transform, &pos);
    ui_transform_inline_reverse_adj_vector_2(&env->m_transform, &pos);
    return pos;
}

ui_vector_2 ui_sprite_render_env_world_to_screen(ui_sprite_render_env_t env, ui_vector_2_t i_pos) {
    ui_vector_2 pos = *i_pos;
    ui_transform_inline_adj_vector_2(&env->m_transform, &pos);
    ui_transform_inline_adj_vector_2(&env->m_base_transform, &pos);
    return pos;
}

ui_vector_2 ui_sprite_render_env_screen_to_logic(ui_sprite_render_env_t env, ui_vector_2_t i_pos) {
    ui_vector_2 pos = * i_pos;
    ui_transform_inline_reverse_adj_vector_2(&env->m_base_transform, &pos);
    return pos;
}

ui_vector_2 ui_sprite_render_env_logic_to_screen(ui_sprite_render_env_t env, ui_vector_2_t i_pos) {
    ui_vector_2 pos = *i_pos;
    ui_transform_inline_adj_vector_2(&env->m_base_transform, &pos);
    return pos;
}

ui_vector_2 ui_sprite_render_env_logic_to_world(ui_sprite_render_env_t env, ui_vector_2_t i_pos) {
    ui_vector_2 pos = *i_pos;
    ui_transform_inline_reverse_adj_vector_2(&env->m_transform, &pos);
    return pos;
}

ui_vector_2 ui_sprite_render_env_world_to_logic(ui_sprite_render_env_t env, ui_vector_2_t i_pos) {
    ui_vector_2 pos = *i_pos;
    ui_transform_inline_adj_vector_2(&env->m_transform, &pos);
    return pos;
}

int ui_sprite_render_env_add_touch_processor(ui_sprite_render_env_t env, ui_sprite_render_env_touch_process_fun_t process_fun, void * process_ctx) {
    ui_sprite_render_env_touch_processor_t processor = ui_sprite_render_env_touch_processor_create(env, process_fun, process_ctx);
    return processor ? 0 : -1;
}

int ui_sprite_render_env_remove_touch_processor(ui_sprite_render_env_t env, ui_sprite_render_env_touch_process_fun_t process_fun, void * process_ctx) {
    ui_sprite_render_env_touch_processor_t processor;

    TAILQ_FOREACH(processor, &env->m_touch_processors, m_next) {
        if (processor->m_process_fun == process_fun && processor->m_process_ctx == process_ctx) {
            ui_sprite_render_env_touch_processor_free(processor);
            return 0;
        }
    }

    return -1;
}

int ui_sprite_render_env_add_transform_monitor(ui_sprite_render_env_t env, ui_sprite_render_env_transform_monitor_fun_t process_fun, void * process_ctx) {
    ui_sprite_render_env_transform_monitor_t processor = ui_sprite_render_env_transform_monitor_create(env, process_fun, process_ctx);
    return processor ? 0 : -1;
}

int ui_sprite_render_env_remove_transform_monitor(ui_sprite_render_env_t env, ui_sprite_render_env_transform_monitor_fun_t process_fun, void * process_ctx) {
    ui_sprite_render_env_transform_monitor_t processor;

    TAILQ_FOREACH(processor, &env->m_transform_monitors, m_next) {
        if (processor->m_process_fun == process_fun && processor->m_process_ctx == process_ctx) {
            ui_sprite_render_env_transform_monitor_free(processor);
            return 0;
        }
    }

    return -1;
}

static void ui_sprite_render_env_update(ui_sprite_world_t world, void * ctx, float delta_s) {
    ui_sprite_render_env_t env = (ui_sprite_render_env_t)ctx;
    ui_sprite_render_layer_t layer;
    ui_sprite_render_anim_t anim, anim_next;

    TAILQ_FOREACH(layer, &env->m_layers, m_next) {
        for(anim = TAILQ_FIRST(&layer->m_anims); anim; anim = anim_next) {
            anim_next = TAILQ_NEXT(anim, m_next_for_layer);
            
            ui_runtime_render_obj_ref_update(anim->m_render_obj_ref, delta_s);
            if (anim->m_auto_remove) {
                if (!ui_runtime_render_obj_is_playing(ui_runtime_render_obj_ref_obj(anim->m_render_obj_ref))) {
                    ui_sprite_render_anim_free(anim);
                }
            }
        }
    }
}

int ui_sprite_render_env_regist(ui_sprite_render_module_t module) {
    if (ui_sprite_cfg_loader_add_resource_loader(
            module->m_loader, UI_SPRITE_RENDER_ENV_NAME, ui_sprite_render_env_res_load, module)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: %s register: register loader fail",
            ui_sprite_render_module_name(module), UI_SPRITE_RENDER_ENV_NAME);
        return -1;
    }

    return 0;
}

void ui_sprite_render_env_unregist(ui_sprite_render_module_t module) {
    if (ui_sprite_cfg_loader_remove_resource_loader(module->m_loader, UI_SPRITE_RENDER_ENV_NAME) != 0) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: loader remove fail",
            ui_sprite_render_module_name(module), UI_SPRITE_RENDER_ENV_NAME);
    }
}

const char * UI_SPRITE_RENDER_ENV_NAME = "RenderEnv";
