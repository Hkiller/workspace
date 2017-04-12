#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_ctrl_track_follow_i.h"
#include "ui_sprite_ctrl_track_mgr_i.h"
#include "ui_sprite_ctrl_track_i.h"
#include "ui_sprite_ctrl_module_i.h"

ui_sprite_ctrl_track_follow_t
ui_sprite_ctrl_track_follow_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CTRL_TRACK_FOLLOW_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ctrl_track_follow_free(ui_sprite_ctrl_track_follow_t show_ctrl) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_ctrl));
}

static void ui_sprite_ctrl_track_follow_on_start(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_ctrl_track_follow_t track_follow = ctx;
    ui_sprite_ctrl_module_t module = track_follow->m_module;
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_ctrl_track_mgr_t track_mgr = ui_sprite_ctrl_track_mgr_find(world);
    ui_sprite_ctrl_track_t track;
	UI_SPRITE_EVT_CTRL_TRACK_FOLLOW_START const * evt_data = evt->data;
    uint8_t pos_of_entity;
    ui_vector_2 pt;

    pos_of_entity = ui_sprite_2d_transform_pos_policy_from_str(evt_data->pos_of_entity);
    if (pos_of_entity == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ctrl-track-follow: on start: pos of entity %s error!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->pos_of_entity);
        return;
    }

    if (track_mgr == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ctrl-track-follow: on start: no track mgr!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    track = ui_sprite_ctrl_track_find(track_mgr, evt_data->track_name);

    if (evt_data->force_create) {
        if (evt_data->track_type[0] == 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): ctrl-track-follow: on start: track %s force create, type not configured!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->track_name);
            return;
        }

        if (track) {
            ui_sprite_ctrl_track_free(track);
            track = NULL;
        }
    }

    if (track == NULL && evt_data->track_type[0]) {
        track = ui_sprite_ctrl_track_create(track_mgr, evt_data->track_name, evt_data->track_type);
        if (track == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): ctrl-track-follow: on start: track %s create with type  exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->track_name);
            return;
        }
    }

    if (track == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ctrl-track-follow: on start: track %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->track_name);
        return;
    }

    pt = ui_sprite_2d_transform_world_pos(transform, pos_of_entity, UI_SPRITE_2D_TRANSFORM_POS_ADJ_ALL);

    if (ui_sprite_ctrl_track_add_point(track, pt) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ctrl-track-follow: on start: track %s add point (%f,%f) fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->track_name, pt.x, pt.y);
        return;
    }
    
    if (evt_data->is_show) {
        if (!ui_sprite_ctrl_track_is_show(track)) {
            if (ui_sprite_ctrl_track_show(track) != 0) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): ctrl-track-follow: on start: track %s show fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->track_name);
                return;
            }
        }
    }

    cpe_str_dup(track_follow->m_track_name, sizeof(track_follow->m_track_name), evt_data->track_name);
    track_follow->m_track_pos = pos_of_entity;

    ui_sprite_fsm_action_sync_update(action, 1);
}

static void ui_sprite_ctrl_track_follow_on_stop(void * ctx, ui_sprite_event_t evt) {
	ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
    ui_sprite_fsm_action_sync_update(action, 0);
}

int ui_sprite_ctrl_track_follow_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ctrl_module_t module = ctx;
	ui_sprite_ctrl_track_follow_t track_follow = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_ctrl_track_follow_start", ui_sprite_ctrl_track_follow_on_start, track_follow) != 0
        || ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_ctrl_track_follow_stop", ui_sprite_ctrl_track_follow_on_stop, track_follow) != 0
        )
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ctrl-track-follow: enter: add eventer handler fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    return 0;
}

void ui_sprite_ctrl_track_follow_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_ctrl_module_t module = ctx;
	ui_sprite_ctrl_track_follow_t track_follow = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
	ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
	ui_sprite_world_t world = ui_sprite_entity_world(entity);
	ui_sprite_ctrl_track_mgr_t track_mgr = ui_sprite_ctrl_track_mgr_find(world);
    ui_sprite_ctrl_track_t track;
    ui_vector_2 pt;

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): track follow: update: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    track = ui_sprite_ctrl_track_find(track_mgr, track_follow->m_track_name);
    if (track == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): track follow: update: track %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), track_follow->m_track_name);
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    pt = ui_sprite_2d_transform_world_pos(transform, track_follow->m_track_pos, UI_SPRITE_2D_TRANSFORM_POS_ADJ_ALL);
    if (ui_sprite_ctrl_track_add_point(track, pt) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ctrl-track-follow: on start: track %s add point (%f,%f) fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), track_follow->m_track_name, pt.x, pt.y);
        return;
    }
}

void ui_sprite_ctrl_track_follow_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

int ui_sprite_ctrl_track_follow_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_ctrl_track_follow_t track_follow = ui_sprite_fsm_action_data(fsm_action);

	bzero(track_follow, sizeof(*track_follow));
	track_follow->m_module = ctx;
    track_follow->m_track_pos = UI_SPRITE_2D_TRANSFORM_POS_ORIGIN;

	return 0;
}

int ui_sprite_ctrl_track_follow_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_ctrl_track_follow_t to_track_follow_to = ui_sprite_fsm_action_data(to);
    ui_sprite_ctrl_track_follow_t from_track_follow_to = ui_sprite_fsm_action_data(from);

    ui_sprite_ctrl_track_follow_init(to, ctx);

    to_track_follow_to->m_track_pos = from_track_follow_to->m_track_pos;

    return 0;
}

void ui_sprite_ctrl_track_follow_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static ui_sprite_fsm_action_t ui_sprite_ctrl_track_follow_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_ctrl_module_t module = ctx;
	ui_sprite_ctrl_track_follow_t track_follow = ui_sprite_ctrl_track_follow_create(fsm_state, name);

	if (track_follow == NULL) {
		CPE_ERROR(module->m_em, "%s: create ctrl_track_follow action: create fail!", ui_sprite_ctrl_module_name(module));
		return NULL;
	}

	return ui_sprite_fsm_action_from_data(track_follow);
}

int ui_sprite_ctrl_track_follow_regist(ui_sprite_ctrl_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CTRL_TRACK_FOLLOW_NAME, sizeof(struct ui_sprite_ctrl_track_follow));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_ctrl_module_name(module), UI_SPRITE_CTRL_TRACK_FOLLOW_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ctrl_track_follow_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ctrl_track_follow_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ctrl_track_follow_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ctrl_track_follow_exit, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ctrl_track_follow_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_ctrl_track_follow_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_CTRL_TRACK_FOLLOW_NAME, ui_sprite_ctrl_track_follow_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_ctrl_track_follow_unregist(ui_sprite_ctrl_module_t module) {
	ui_sprite_fsm_action_meta_t meta;

	meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CTRL_TRACK_FOLLOW_NAME);
	if (meta == NULL) {
		CPE_ERROR(
			module->m_em, "%s: %s unregister: meta not exist",
			ui_sprite_ctrl_module_name(module), UI_SPRITE_CTRL_TRACK_FOLLOW_NAME);
		return;
	}

	ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CTRL_TRACK_FOLLOW_NAME);
    }
}

const char * UI_SPRITE_CTRL_TRACK_FOLLOW_NAME = "track-follow";
