#include "cpe/pal/pal_strings.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_ctrl_track_manip_i.h"
#include "ui_sprite_ctrl_module_i.h"

ui_sprite_ctrl_track_manip_t
ui_sprite_ctrl_track_manip_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CTRL_TRACK_MANIP_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ctrl_track_manip_free(ui_sprite_ctrl_track_manip_t show_ctrl) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_ctrl));
}

uint8_t ui_sprite_ctrl_track_manip_pos(ui_sprite_ctrl_track_manip_t track_manip) {
    return track_manip->m_track_pos;
}

void ui_sprite_ctrl_track_manip_set_pos(ui_sprite_ctrl_track_manip_t track_manip, uint8_t pos_policy) {
    track_manip->m_track_pos = pos_policy;
}

int ui_sprite_ctrl_track_manip_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ctrl_module_t module = ctx;
	ui_sprite_ctrl_track_manip_t track_manip = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): track flip: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    ui_sprite_fsm_action_start_update(fsm_action);

    track_manip->m_pre_pos = ui_sprite_2d_transform_world_pos(transform, track_manip->m_track_pos, UI_SPRITE_2D_TRANSFORM_POS_ADJ_ALL);

    return 0;
}

void ui_sprite_ctrl_track_manip_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_ctrl_module_t module = ctx;
	ui_sprite_ctrl_track_manip_t track_manip = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
	ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): track flip: update: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    ui_sprite_2d_transform_world_pos(transform, track_manip->m_track_pos, UI_SPRITE_2D_TRANSFORM_POS_ADJ_ALL);
}

void ui_sprite_ctrl_track_manip_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

int ui_sprite_ctrl_track_manip_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_ctrl_track_manip_t track_manip = ui_sprite_fsm_action_data(fsm_action);

	bzero(track_manip, sizeof(*track_manip));
	track_manip->m_module = ctx;
    track_manip->m_track_pos = UI_SPRITE_2D_TRANSFORM_POS_ORIGIN;

	return 0;
}

int ui_sprite_ctrl_track_manip_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_ctrl_track_manip_t to_track_manip_to = ui_sprite_fsm_action_data(to);
    ui_sprite_ctrl_track_manip_t from_track_manip_to = ui_sprite_fsm_action_data(from);

    ui_sprite_ctrl_track_manip_init(to, ctx);

    to_track_manip_to->m_track_pos = from_track_manip_to->m_track_pos;

    return 0;
}

void ui_sprite_ctrl_track_manip_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static ui_sprite_fsm_action_t ui_sprite_ctrl_track_manip_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_ctrl_module_t module = ctx;
	ui_sprite_ctrl_track_manip_t track_manip = ui_sprite_ctrl_track_manip_create(fsm_state, name);

	if (track_manip == NULL) {
		CPE_ERROR(module->m_em, "%s: create ctrl_track_manip action: create fail!", ui_sprite_ctrl_module_name(module));
		return NULL;
	}

	return ui_sprite_fsm_action_from_data(track_manip);
}

int ui_sprite_ctrl_track_manip_regist(ui_sprite_ctrl_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CTRL_TRACK_MANIP_NAME, sizeof(struct ui_sprite_ctrl_track_manip));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_ctrl_module_name(module), UI_SPRITE_CTRL_TRACK_MANIP_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ctrl_track_manip_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ctrl_track_manip_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ctrl_track_manip_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ctrl_track_manip_exit, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ctrl_track_manip_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_ctrl_track_manip_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_CTRL_TRACK_MANIP_NAME, ui_sprite_ctrl_track_manip_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_ctrl_track_manip_unregist(ui_sprite_ctrl_module_t module) {
	ui_sprite_fsm_action_meta_t meta;

	meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CTRL_TRACK_MANIP_NAME);
	if (meta == NULL) {
		CPE_ERROR(
			module->m_em, "%s: %s unregister: meta not exist",
			ui_sprite_ctrl_module_name(module), UI_SPRITE_CTRL_TRACK_MANIP_NAME);
		return;
	}

	ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CTRL_TRACK_MANIP_NAME);
    }
}

const char * UI_SPRITE_CTRL_TRACK_MANIP_NAME = "track-manip";
