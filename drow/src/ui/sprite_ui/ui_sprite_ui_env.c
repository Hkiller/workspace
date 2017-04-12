#include <assert.h>
#include "stdio.h"
#include "cpe/pal/pal_strings.h"
#include "gd/app/app_context.h"
#include "plugin/ui/plugin_ui_popup.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_basic/ui_sprite_basic_noop.h"
#include "ui_sprite_ui_env_i.h"
#include "ui_sprite_ui_popup_def_i.h"
#include "ui_sprite_ui_phase_i.h"
#include "ui_sprite_ui_state_i.h"
#include "ui_sprite_ui_navigation_i.h"
#include "ui_sprite_ui_page_eh_i.h"

static void ui_sprite_ui_env_update(ui_sprite_world_t world, void * ctx, float delta_s);
static int ui_sprite_ui_env_init_entity(ui_sprite_ui_env_t env);
static void ui_sprite_ui_env_clear(ui_sprite_world_res_t world_res, void * ctx) {
    ui_sprite_world_t world = ui_sprite_world_res_world(world_res);
    ui_sprite_ui_env_t env = (ui_sprite_ui_env_t)ui_sprite_world_res_data(world_res);
    struct plugin_ui_popup_it popup_it;
    plugin_ui_popup_t popup;
    
    ui_sprite_world_remove_updator(world, env);
    
    assert(env->m_module->m_env == env);

    plugin_ui_env_popups(env->m_env, &popup_it);
    while((popup = plugin_ui_popup_it_next((&popup_it)))) {
        plugin_ui_popup_free(popup);
    }
    
    plugin_ui_env_clear_runtime(env->m_env);

    ui_sprite_entity_free(env->m_entity);
    env->m_entity = NULL;

    plugin_ui_env_free(env->m_env);
    env->m_env = NULL;

    env->m_module->m_env = NULL;
}

ui_sprite_ui_env_t
ui_sprite_ui_env_create(ui_sprite_ui_module_t module, ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res;
    ui_sprite_ui_env_t env;

    if (module->m_env != NULL) {
        CPE_ERROR(module->m_em, "create ui env: ui env already exist!");
        return NULL;
    }

    world_res =
        ui_sprite_world_res_create(
            world, UI_SPRITE_UI_ENV_NAME, sizeof(struct ui_sprite_ui_env));

    if (world_res == NULL) {
        CPE_ERROR(module->m_em, "create ui env: creat res fail!");
        return NULL;
    }

    env = (ui_sprite_ui_env_t)ui_sprite_world_res_data(world_res);

    bzero(env, sizeof(*env));

    env->m_module = module;

    env->m_backend.ctx = env;
    /*event*/
    env->m_backend.send_event = ui_sprite_ui_env_send_event;
    env->m_backend.build_and_send_event = ui_sprite_ui_env_build_and_send_event;
    /*popup_def*/
    env->m_backend.popup_def_capacity = sizeof(struct ui_sprite_ui_popup_def);
    env->m_backend.popup_def_init = ui_sprite_ui_env_popup_def_init;
    env->m_backend.popup_def_fini = ui_sprite_ui_env_popup_def_fini;    
    /*popup*/
    env->m_backend.popup_enter = ui_sprite_ui_env_popup_enter;
    env->m_backend.popup_leave = ui_sprite_ui_env_popup_leave;
    /*phase*/
    env->m_backend.phase_capacity = sizeof(struct ui_sprite_ui_phase);
    env->m_backend.phase_init = ui_sprite_ui_env_phase_init;
    env->m_backend.phase_fini = ui_sprite_ui_env_phase_fini;    
    env->m_backend.phase_enter = ui_sprite_ui_env_phase_enter;
    env->m_backend.phase_leave = ui_sprite_ui_env_phase_leave;
    /*state*/
    env->m_backend.state_capacity = sizeof(struct ui_sprite_ui_state);
    env->m_backend.state_init = ui_sprite_ui_env_state_init;
    env->m_backend.state_fini = ui_sprite_ui_env_state_fini;    
    env->m_backend.state_node_active = ui_sprite_ui_env_state_node_active;
    env->m_backend.state_node_is_active = ui_sprite_ui_env_state_node_is_active;
    env->m_backend.state_node_deactive = ui_sprite_ui_env_state_node_deactive;
    /*navigation*/
    env->m_backend.navigation_capacity = sizeof(struct ui_sprite_ui_navigation);
    env->m_backend.navigation_init = ui_sprite_ui_env_navigation_init;
    env->m_backend.navigation_fini = ui_sprite_ui_env_navigation_fini;    
    /*eh*/
    env->m_backend.eh_capacity = sizeof(struct ui_sprite_ui_page_eh);
    env->m_backend.eh_init = ui_sprite_ui_page_eh_init;
    env->m_backend.eh_fini = ui_sprite_ui_page_eh_fini;
    env->m_backend.eh_active = ui_sprite_ui_page_eh_active;
    env->m_backend.eh_deactive = ui_sprite_ui_page_eh_deactive;

    if (ui_sprite_world_add_updator(world, ui_sprite_ui_env_update, env) != 0
        || ui_sprite_world_set_updator_priority(world, env, (int8_t)-255) != 0)
    {
        CPE_ERROR(module->m_em, "create ui env: add updator fail!");
        ui_sprite_world_res_free(world_res);
        return NULL;
    }

    env->m_env = plugin_ui_env_create(module->m_ui_module);
    if (env->m_env == NULL) {
        CPE_ERROR(module->m_em, "create ui env: creat plugin env fail!");
        ui_sprite_world_remove_updator(world, env);
        ui_sprite_world_res_free(world_res);
        return NULL;
    }
    plugin_ui_env_set_backend(env->m_env, &env->m_backend);

    env->m_entity = ui_sprite_entity_create(world, "ui", NULL);
    if (env->m_entity == NULL) {
        CPE_ERROR(module->m_em, "create ui env: creat entity fail!");
        plugin_ui_env_free(env->m_env);
        ui_sprite_world_remove_updator(world, env);
        ui_sprite_world_res_free(world_res);
        return NULL;
    }

    if (ui_sprite_ui_env_init_entity(env) != 0) {
        CPE_ERROR(module->m_em, "create ui env: entity init fail!");
        ui_sprite_entity_free(env->m_entity);
        plugin_ui_env_free(env->m_env);
        ui_sprite_world_remove_updator(world, env);
        ui_sprite_world_res_free(world_res);
        return NULL;
    }
    
    if (ui_sprite_entity_enter(env->m_entity) != 0) {
        CPE_ERROR(module->m_em, "create ui env: entity enter fail!");
        ui_sprite_entity_free(env->m_entity);
        plugin_ui_env_free(env->m_env);
        ui_sprite_world_remove_updator(world, env);
        ui_sprite_world_res_free(world_res);
        return NULL;
    }
    
    module->m_env = env;
    
    ui_sprite_world_res_set_free_fun(world_res, ui_sprite_ui_env_clear, NULL);

    return env;
}

void ui_sprite_ui_env_free(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_UI_ENV_NAME);
    if (world_res) {
        ui_sprite_world_res_free(world_res);
    }
}

static int ui_sprite_ui_env_init_entity(ui_sprite_ui_env_t env) {
    ui_sprite_component_t component;
    ui_sprite_fsm_ins_t fsm;
    ui_sprite_fsm_state_t fsm_state;
    ui_sprite_basic_noop_t keep_state_action;
    
    component = ui_sprite_component_create(env->m_entity, UI_SPRITE_FSM_COMPONENT_FSM_NAME);
    if (component == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_init_entity: create %s component fail!",
            UI_SPRITE_FSM_COMPONENT_FSM_NAME);
        return -1;
    }

    fsm = ui_sprite_component_data(component);

    fsm_state = ui_sprite_fsm_state_create(fsm, "R");
    if (fsm_state == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_init_entity: create fsm state fail!");
        return -1;
    }

    if (ui_sprite_fsm_set_default_state(fsm, "R") != 0) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_init_entity: set default state fail!");
        return -1;
    }

    keep_state_action = ui_sprite_basic_noop_create(fsm_state, "keep-state");
    if (keep_state_action == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_init_entity: create keep state action fail!");
        return -1;
    }

    if (ui_sprite_fsm_action_set_life_circle(
            ui_sprite_fsm_action_from_data(keep_state_action), ui_sprite_fsm_action_life_circle_endless)
        != 0)
    {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_init_entity: set keep state action life_circle fail!");
        return -1;
    }

    return 0;
}

static void ui_sprite_ui_env_update(ui_sprite_world_t world, void * ctx, float delta_s) {
    ui_sprite_ui_env_t env = (ui_sprite_ui_env_t)ctx;
    plugin_ui_env_update(env->m_env, delta_s);    
}

ui_sprite_ui_env_t ui_sprite_ui_env_find(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_UI_ENV_NAME);
    return world_res ? (ui_sprite_ui_env_t)ui_sprite_world_res_data(world_res) : NULL;
}

ui_sprite_ui_env_t ui_sprite_ui_env_get(ui_sprite_ui_module_t module) {
    return module->m_env;
}

plugin_ui_env_t ui_sprite_ui_env_env(ui_sprite_ui_env_t env) {
    return env->m_env;
}

ui_sprite_entity_t ui_sprite_ui_env_entity(ui_sprite_ui_env_t env) {
    return env->m_entity;
}

void ui_sprite_ui_env_set_debug(ui_sprite_ui_env_t env, uint8_t debug) {
    plugin_ui_env_set_debug(env->m_env, debug);
    ui_sprite_entity_set_debug(env->m_entity, debug >= 1 ? debug - 1 : 0);
}

int ui_sprite_ui_env_regist(ui_sprite_ui_module_t module) {
    if (ui_sprite_cfg_loader_add_resource_loader(
            module->m_loader, UI_SPRITE_UI_ENV_NAME, ui_sprite_ui_env_res_load, module)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: %s register: register loader fail",
            ui_sprite_ui_module_name(module), UI_SPRITE_UI_ENV_NAME);
        return -1;
    }

    return 0;
}

void ui_sprite_ui_env_unregist(ui_sprite_ui_module_t module) {
    if (ui_sprite_cfg_loader_remove_resource_loader(module->m_loader, UI_SPRITE_UI_ENV_NAME) != 0) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: loader remove fail",
            ui_sprite_ui_module_name(module), UI_SPRITE_UI_ENV_NAME);
    }
}

const char * UI_SPRITE_UI_ENV_NAME = "UIEnv";
