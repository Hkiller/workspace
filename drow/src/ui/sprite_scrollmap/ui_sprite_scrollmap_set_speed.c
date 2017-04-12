#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "plugin/scrollmap/plugin_scrollmap_env.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_scrollmap_set_speed_i.h"
#include "ui_sprite_scrollmap_env_i.h"

ui_sprite_scrollmap_set_speed_t ui_sprite_scrollmap_set_speed_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_SCROLLMAP_SET_SPEED_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_scrollmap_set_speed_free(ui_sprite_scrollmap_set_speed_t set_speed) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(set_speed);
    ui_sprite_fsm_action_free(fsm_action);
}

void ui_sprite_scrollmap_set_speed_set_to_speed(ui_sprite_scrollmap_set_speed_t set_speed, float to_speed) {
    set_speed->m_to_speed = to_speed;
}

float ui_sprite_scrollmap_set_speed_to_spped(ui_sprite_scrollmap_set_speed_t set_speed) {
    return set_speed->m_to_speed;
}

void ui_sprite_scrollmap_set_speed_set_acceleration(ui_sprite_scrollmap_set_speed_t set_speed, float acceleration) {
    set_speed->m_acceleration = acceleration;
}

float ui_sprite_scrollmap_set_speed_acceleration(ui_sprite_scrollmap_set_speed_t set_speed) {
    return set_speed->m_acceleration;
}

static int ui_sprite_scrollmap_set_speed_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_scrollmap_module_t module = ctx;
    ui_sprite_scrollmap_set_speed_t set_speed = ui_sprite_fsm_action_data(fsm_action);

    if (set_speed->m_acceleration <= 0.0f) {
        ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
        ui_sprite_world_t world = ui_sprite_entity_world(entity);
        ui_sprite_scrollmap_env_t scene_env;

        scene_env = ui_sprite_scrollmap_env_find(world);
        if (scene_env == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): set scene speed: no scene env!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

        plugin_scrollmap_env_set_move_speed(scene_env->m_env, set_speed->m_to_speed);
        return 0;
    }
    else {
        return ui_sprite_fsm_action_start_update(fsm_action);
    }
}

static void ui_sprite_scrollmap_set_speed_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static void ui_sprite_scrollmap_set_speed_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_scrollmap_module_t module = ctx;
    ui_sprite_scrollmap_set_speed_t set_speed = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_scrollmap_env_t scene_env;
    float cur_speed;
    
    scene_env = ui_sprite_scrollmap_env_find(world);
    if (scene_env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): set scene speed: no scene env!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    if (set_speed->m_acceleration <= 0.0f) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): set scene speed: acceleration %f error!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), set_speed->m_acceleration);
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    cur_speed = plugin_scrollmap_env_move_speed(scene_env->m_env);
    if (cur_speed > set_speed->m_to_speed) {
        float delat_speed = cur_speed - set_speed->m_to_speed;
        float acceleration_delta_speed = set_speed->m_acceleration * delta_s;

        if (delat_speed > acceleration_delta_speed) {
            plugin_scrollmap_env_set_move_speed(scene_env->m_env, cur_speed - acceleration_delta_speed);
        }
        else {
            plugin_scrollmap_env_set_move_speed(scene_env->m_env, set_speed->m_to_speed);
        }
    }
    else if (cur_speed < set_speed->m_to_speed) {
        float delat_speed = cur_speed - set_speed->m_to_speed;
        float acceleration_delta_speed = set_speed->m_acceleration * delta_s;

        if (delat_speed > acceleration_delta_speed) {
            plugin_scrollmap_env_set_move_speed(scene_env->m_env, cur_speed + acceleration_delta_speed);
        }
        else {
            plugin_scrollmap_env_set_move_speed(scene_env->m_env, set_speed->m_to_speed);
        }
    }

    if (plugin_scrollmap_env_move_speed(scene_env->m_env) == set_speed->m_to_speed) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static int ui_sprite_scrollmap_set_speed_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_scrollmap_set_speed_t set_speed = ui_sprite_fsm_action_data(fsm_action);
    set_speed->m_module = ctx;
	set_speed->m_to_speed = 0.0f;
	set_speed->m_acceleration = 0.0f;
    return 0;
}

static void ui_sprite_scrollmap_set_speed_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_scrollmap_set_speed_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_scrollmap_set_speed_t to_set_speed = ui_sprite_fsm_action_data(to);
    ui_sprite_scrollmap_set_speed_t from_set_speed = ui_sprite_fsm_action_data(from);

    if (ui_sprite_scrollmap_set_speed_init(to, ctx)) return -1;

    to_set_speed->m_to_speed = from_set_speed->m_to_speed;
    to_set_speed->m_acceleration = from_set_speed->m_acceleration;

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_scrollmap_set_speed_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_scrollmap_module_t module = ctx;
    ui_sprite_scrollmap_set_speed_t set_speed = ui_sprite_scrollmap_set_speed_create(fsm_state, name);
    
    if (set_speed == NULL) {
        CPE_ERROR(module->m_em, "%s: create set_speed action: create fail!", ui_sprite_scrollmap_module_name(module));
        return NULL;
    }

    set_speed->m_to_speed = cfg_get_float(cfg, "to-speed", 0.0f);
    set_speed->m_acceleration = cfg_get_float(cfg, "acceleration", 0.0f);

    return ui_sprite_fsm_action_from_data(set_speed);
}

int ui_sprite_scrollmap_set_speed_regist(ui_sprite_scrollmap_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_SCROLLMAP_SET_SPEED_NAME, sizeof(struct ui_sprite_scrollmap_set_speed));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: moving enable emitter register: meta create fail",
            ui_sprite_scrollmap_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_scrollmap_set_speed_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_scrollmap_set_speed_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_scrollmap_set_speed_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_scrollmap_set_speed_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_scrollmap_set_speed_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_scrollmap_set_speed_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_SCROLLMAP_SET_SPEED_NAME, ui_sprite_scrollmap_set_speed_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_scrollmap_set_speed_unregist(ui_sprite_scrollmap_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_SCROLLMAP_SET_SPEED_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_scrollmap_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_SCROLLMAP_SET_SPEED_NAME);
}

const char * UI_SPRITE_SCROLLMAP_SET_SPEED_NAME = "scrollmap-set-speed";
