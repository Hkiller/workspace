#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "gd/app/app_context.h"
#include "render/utils/ui_rect.h"
#include "plugin/scrollmap/plugin_scrollmap_env.h"
#include "plugin/scrollmap/plugin_scrollmap_obj.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_world_calc.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_render/ui_sprite_render_env.h"
#include "ui_sprite_scrollmap_env_i.h"
#include "ui_sprite_scrollmap_obj_factory_i.h"
#include "plugin/scrollmap/plugin_scrollmap_script.h"

#ifdef __cplusplus
extern "C" {
#endif

static void ui_sprite_scrollmap_env_update(ui_sprite_world_t world, void * ctx, float delta_s);
static uint8_t ui_sprite_scrollmap_script_check(void * ctx, plugin_scrollmap_layer_t layer, SCROLLMAP_SCRIPT const * script);
    
static void ui_sprite_scrollmap_env_clear(ui_sprite_world_res_t world_res, void * ctx) {
    ui_sprite_scrollmap_env_t env = (ui_sprite_scrollmap_env_t)ui_sprite_world_res_data(world_res);
    plugin_scrollmap_env_free(env->m_env);
    ui_sprite_world_remove_updator(ui_sprite_world_res_world(world_res), env);
}

ui_sprite_scrollmap_env_t
ui_sprite_scrollmap_env_create(
    ui_sprite_scrollmap_module_t module, ui_sprite_world_t world,
    plugin_scrollmap_moving_way_t moving_way, ui_vector_2_t base_size)
{
    ui_sprite_world_res_t world_res;
    ui_sprite_scrollmap_env_t env;

    world_res =
        ui_sprite_world_res_create(
            world, UI_SPRITE_SCROLLMAP_ENV_NAME, sizeof(struct ui_sprite_scrollmap_env));

    if (world_res == NULL) {
        CPE_ERROR(module->m_em, "create scrollmap env: creat res fail!");
        return NULL;
    }

    env = (ui_sprite_scrollmap_env_t)ui_sprite_world_res_data(world_res);

    env->m_module = module;
    env->m_entity_index = 0;
    env->m_debug = 0;
    env->m_runtime_size_policy = ui_sprite_scrollmap_runtime_size_no_adj;
    env->m_is_init = 0;
    
    env->m_env = plugin_scrollmap_env_create(module->m_scrollmap_module, moving_way, base_size);
    if (env->m_env == NULL) {
        CPE_ERROR(module->m_em, "create scrollmap env: creat plugin scrollmap env fail!");
        ui_sprite_world_res_free(world_res);
        return NULL;
    }

    if (plugin_scrollmap_env_set_obj_factory(
            env->m_env,
            env, sizeof(struct ui_sprite_scrollmap_obj_stub),
            ui_sprite_scrollmap_obj_name,
            ui_sprite_scrollmap_obj_on_init,
            ui_sprite_scrollmap_obj_on_update,
            ui_sprite_scrollmap_obj_on_event,
            ui_sprite_scrollmap_obj_on_destory)
        != 0)
    {
        CPE_ERROR(module->m_em, "create scrollmap env: set obj factory fail!");
        plugin_scrollmap_env_free(env->m_env);
        ui_sprite_world_res_free(world_res);
        return NULL;
    }

    if (ui_sprite_world_add_updator(world, ui_sprite_scrollmap_env_update, env) != 0) {
        CPE_ERROR(module->m_em, "create scrollmap env: add updator fail!");
        plugin_scrollmap_env_free(env->m_env);
        ui_sprite_world_res_free(world_res);
        return NULL;
    }

    plugin_scrollmap_env_set_script_filter(env->m_env, env, ui_sprite_scrollmap_script_check);
    
    ui_sprite_world_res_set_free_fun(world_res, ui_sprite_scrollmap_env_clear, NULL);

    return env;
}

void ui_sprite_scrollmap_env_free(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_SCROLLMAP_ENV_NAME);
    if (world_res) {
        ui_sprite_world_res_free(world_res);
    }
}

ui_sprite_scrollmap_env_t ui_sprite_scrollmap_env_find(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_SCROLLMAP_ENV_NAME);
    return world_res ? (ui_sprite_scrollmap_env_t)ui_sprite_world_res_data(world_res) : NULL;
}

plugin_scrollmap_env_t ui_sprite_scrollmap_env_env(ui_sprite_scrollmap_env_t env) {
    return env->m_env;
}

ui_sprite_scrollmap_runtime_size_policy_t ui_sprite_scrollmap_env_runtime_size_policy(ui_sprite_scrollmap_env_t env) {
    return env->m_runtime_size_policy;
}
    
void ui_sprite_scrollmap_env_set_runtime_size_policy(ui_sprite_scrollmap_env_t env, ui_sprite_scrollmap_runtime_size_policy_t policy) {
    if (env->m_runtime_size_policy != policy) {
        env->m_runtime_size_policy = policy;
        env->m_is_init = 0;
    }
}

static int ui_sprite_scrollmap_env_adj_env(ui_sprite_scrollmap_env_t env) {
    ui_sprite_world_t world = ui_sprite_world_res_world(ui_sprite_world_res_from_data(env));
    ui_sprite_render_env_t render_env;
    ui_vector_2_t screen_size;
    ui_vector_2_t base_size = plugin_scrollmap_env_base_size(env->m_env);
    ui_vector_2 runing_size;
        
    render_env = ui_sprite_render_env_find(world);
    if (render_env == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_scrollmap_env_adj_env: no render_env!");
        return -1;
    }

    screen_size = ui_sprite_render_env_size(render_env);

    switch(env->m_runtime_size_policy) {
    case ui_sprite_scrollmap_runtime_size_no_adj:
        runing_size = *base_size;
        break;
    case ui_sprite_scrollmap_runtime_size_fix_x:
        runing_size.x = base_size->x;
        runing_size.y = screen_size->y * (base_size->x / screen_size->x);
        break;
    case ui_sprite_scrollmap_runtime_size_fix_y:
        runing_size.y = base_size->y;
        runing_size.x = screen_size->x * (base_size->y / screen_size->y);
        break;
    case ui_sprite_scrollmap_runtime_size_resize:
        runing_size.x = screen_size->x;
        runing_size.y = screen_size->y;
        break;
    default:
        CPE_ERROR(env->m_module->m_em, "ui_sprite_scrollmap_env_adj_env: unknown resize policy %d!", env->m_runtime_size_policy);
        return -1;
    }

    /* printf( */
    /*     "xxxxxx: policy=%d, base-size=(%f,%f), runing-size=(%f,%f) screen-size=(%f,%f)\n", */
    /*     env->m_runtime_size_policy, base_size->x, base_size->y, runing_size.x, runing_size.y, */
    /*     screen_size.x, screen_size.y); */
    
    plugin_scrollmap_env_set_runing_size(env->m_env, &runing_size);

    env->m_is_init = 1;

    return 0;
}

static uint8_t ui_sprite_scrollmap_script_check(void * ctx, plugin_scrollmap_layer_t layer, SCROLLMAP_SCRIPT const * script) {
    ui_sprite_scrollmap_env_t env = ctx;
    
    if (script->condition[0]) {
        ui_sprite_world_res_t res = ui_sprite_world_res_from_data(env);
        ui_sprite_world_t world = ui_sprite_world_res_world(res);
        return ui_sprite_world_calc_bool_with_dft(script->condition, world, NULL, 0);
    }
    else {
        return 1;
    }
}
    
void ui_sprite_scrollmap_env_update(ui_sprite_world_t world, void * ctx, float delta_s) {
    ui_sprite_scrollmap_env_t env = ctx;

    if (!env->m_is_init) {
        if (ui_sprite_scrollmap_env_adj_env(env) != 0) return;
    }
    
    plugin_scrollmap_env_update(env->m_env, delta_s);
}

int ui_sprite_scrollmap_env_set_update_priority(ui_sprite_scrollmap_env_t env, int8_t priority) {
    ui_sprite_world_res_t res = ui_sprite_world_res_from_data(env);
    ui_sprite_world_t world = ui_sprite_world_res_world(res);
    return ui_sprite_world_set_updator_priority(world, env, priority);
}

int ui_sprite_scrollmap_env_regist(ui_sprite_scrollmap_module_t module) {
    if (ui_sprite_cfg_loader_add_resource_loader(
            module->m_loader, UI_SPRITE_SCROLLMAP_ENV_NAME, ui_sprite_scrollmap_env_load, module)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: %s register: register loader fail",
            ui_sprite_scrollmap_module_name(module), UI_SPRITE_SCROLLMAP_ENV_NAME);
        return -1;
    }

    return 0;
}

void ui_sprite_scrollmap_env_unregist(ui_sprite_scrollmap_module_t module) {
    if (ui_sprite_cfg_loader_remove_resource_loader(module->m_loader, UI_SPRITE_SCROLLMAP_ENV_NAME) != 0) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: loader remove fail",
            ui_sprite_scrollmap_module_name(module), UI_SPRITE_SCROLLMAP_ENV_NAME);
    }
}

const char * UI_SPRITE_SCROLLMAP_ENV_NAME = "ScrollMapEnv";

#ifdef __cplusplus
}
#endif
