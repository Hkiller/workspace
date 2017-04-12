#include <math.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_wait_switchback.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_2d_wait_switchback_i.h"
#include "ui_sprite_2d_module_i.h"
#include "protocol/ui/sprite_2d/ui_sprite_2d_evt.h"

ui_sprite_2d_wait_switchback_t
ui_sprite_2d_wait_switchback_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_2D_WAIT_SWITCHBACK_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_2d_wait_switchback_free(ui_sprite_2d_wait_switchback_t show_anim) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_anim));
}

uint8_t ui_sprite_2d_wait_switchback_pos(ui_sprite_2d_wait_switchback_t wait_switchback) {
    return wait_switchback->m_track_pos;
}

void ui_sprite_2d_wait_switchback_set_pos(ui_sprite_2d_wait_switchback_t wait_switchback, uint8_t pos_policy) {
    wait_switchback->m_track_pos = pos_policy;
}

uint8_t ui_sprite_2d_wait_switchback_process_x(ui_sprite_2d_wait_switchback_t wait_switchback) {
    return wait_switchback->m_process_x;
}

void ui_sprite_2d_wait_switchback_set_process_x(ui_sprite_2d_wait_switchback_t wait_switchback, uint8_t process_x) {
    wait_switchback->m_process_x = process_x;
}

uint8_t ui_sprite_2d_wait_switchback_process_y(ui_sprite_2d_wait_switchback_t wait_switchback) {
    return wait_switchback->m_process_y;
}

void ui_sprite_2d_wait_switchback_set_process_y(ui_sprite_2d_wait_switchback_t wait_switchback, uint8_t process_y) {
    wait_switchback->m_process_y = process_y;
}

int ui_sprite_2d_wait_switchback_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_wait_switchback_t wait_switchback = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): track flip: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    ui_sprite_fsm_action_start_update(fsm_action);

    wait_switchback->m_pre_pos = ui_sprite_2d_transform_world_pos(transform, UI_SPRITE_2D_TRANSFORM_POS_ORIGIN, 0);
    wait_switchback->m_moving_x = 0;
    wait_switchback->m_moving_y = 0;

    return 0;
}

void ui_sprite_2d_wait_switchback_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_wait_switchback_t wait_switchback = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
	ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
    ui_vector_2 cur_pos;

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): track flip: update: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    cur_pos = ui_sprite_2d_transform_world_pos(
        transform, wait_switchback->m_track_pos,
        UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_FLIP | UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_ANGLE);

    if (wait_switchback->m_process_x && fabs(cur_pos.x - wait_switchback->m_pre_pos.x) > 0.5f) {
        int8_t moving_x = cur_pos.x > wait_switchback->m_pre_pos.x ? 0 : 1;

        if (wait_switchback->m_moving_x != 0 && wait_switchback->m_moving_x != moving_x) {
            ui_sprite_fsm_action_stop_update(fsm_action);
            return;
        }
        else {
            wait_switchback->m_moving_x = moving_x;
        }
    }

    if (wait_switchback->m_process_y && fabs(cur_pos.y - wait_switchback->m_pre_pos.y) > 0.5f) {
        int8_t moving_y = cur_pos.y < wait_switchback->m_pre_pos.y ? 0 : 1;

        if (wait_switchback->m_moving_y != 0 && wait_switchback->m_moving_y != moving_y) {
            ui_sprite_fsm_action_stop_update(fsm_action);
            return;
        }
        else {
            wait_switchback->m_moving_y = moving_y;
        }
    }
}

void ui_sprite_2d_wait_switchback_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

int ui_sprite_2d_wait_switchback_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_2d_wait_switchback_t wait_switchback = ui_sprite_fsm_action_data(fsm_action);

	bzero(wait_switchback, sizeof(*wait_switchback));
	wait_switchback->m_module = ctx;
    wait_switchback->m_track_pos = UI_SPRITE_2D_TRANSFORM_POS_ORIGIN;

	return 0;
}

int ui_sprite_2d_wait_switchback_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_2d_wait_switchback_t to_wait_switchback_to = ui_sprite_fsm_action_data(to);
    ui_sprite_2d_wait_switchback_t from_wait_switchback_to = ui_sprite_fsm_action_data(from);

    ui_sprite_2d_wait_switchback_init(to, ctx);

    to_wait_switchback_to->m_process_x = from_wait_switchback_to->m_process_x;
    to_wait_switchback_to->m_process_y = from_wait_switchback_to->m_process_y;
    to_wait_switchback_to->m_track_pos = from_wait_switchback_to->m_track_pos;

    return 0;
}

void ui_sprite_2d_wait_switchback_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

ui_sprite_fsm_action_t ui_sprite_2d_wait_switchback_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_wait_switchback_t p2d_wait_switchback = ui_sprite_2d_wait_switchback_create(fsm_state, name);
    const char * track_pos;

	if (p2d_wait_switchback == NULL) {
		CPE_ERROR(module->m_em, "%s: create anim_2d_wait_switchback action: create fail!", ui_sprite_2d_module_name(module));
		return NULL;
	}

    if ((track_pos = cfg_get_string(cfg, "track-pos", NULL))) {
        uint8_t pos_policy = ui_sprite_2d_transform_pos_policy_from_str(track_pos);
        if (pos_policy == 0) {
            CPE_ERROR(
                module->m_em, "%s: create anim_2d_wait_switchback action: track-pos %s is unknown!",
                ui_sprite_2d_module_name(module), track_pos);
            ui_sprite_2d_wait_switchback_free(p2d_wait_switchback);
            return NULL;
        }

        ui_sprite_2d_wait_switchback_set_pos(p2d_wait_switchback, pos_policy);
    }

    ui_sprite_2d_wait_switchback_set_process_x(
        p2d_wait_switchback,
        cfg_get_uint8(cfg, "process-x", ui_sprite_2d_wait_switchback_process_x(p2d_wait_switchback)));

    ui_sprite_2d_wait_switchback_set_process_y(
        p2d_wait_switchback,
        cfg_get_uint8(cfg, "process-y", ui_sprite_2d_wait_switchback_process_y(p2d_wait_switchback)));

	return ui_sprite_fsm_action_from_data(p2d_wait_switchback);
}

int ui_sprite_2d_wait_switchback_regist(ui_sprite_2d_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_2D_WAIT_SWITCHBACK_NAME, sizeof(struct ui_sprite_2d_wait_switchback));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_WAIT_SWITCHBACK_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_2d_wait_switchback_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_2d_wait_switchback_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_2d_wait_switchback_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_2d_wait_switchback_exit, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_2d_wait_switchback_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_2d_wait_switchback_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_2D_WAIT_SWITCHBACK_NAME, ui_sprite_2d_wait_switchback_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_2d_wait_switchback_unregist(ui_sprite_2d_module_t module) {
	ui_sprite_fsm_action_meta_t meta;

	meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_2D_WAIT_SWITCHBACK_NAME);
	if (meta == NULL) {
		CPE_ERROR(
			module->m_em, "%s: %s unregister: meta not exist",
			ui_sprite_2d_module_name(module), UI_SPRITE_2D_WAIT_SWITCHBACK_NAME);
		return;
	}

	ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_2D_WAIT_SWITCHBACK_NAME);
    }
}

const char * UI_SPRITE_2D_WAIT_SWITCHBACK_NAME = "2d-wait-switchback";
