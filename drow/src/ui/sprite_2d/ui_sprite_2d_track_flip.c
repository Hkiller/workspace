#include <math.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_track_flip.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_2d_track_flip_i.h"
#include "ui_sprite_2d_module_i.h"
#include "ui_sprite_2d_transform_i.h"
#include "protocol/ui/sprite_2d/ui_sprite_2d_evt.h"

ui_sprite_2d_track_flip_t
ui_sprite_2d_track_flip_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_2D_TRACK_FLIP_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_2d_track_flip_free(ui_sprite_2d_track_flip_t show_anim) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_anim));
}

uint8_t ui_sprite_2d_track_flip_process_x(ui_sprite_2d_track_flip_t track_flip) {
    return track_flip->m_process_x;
}

void ui_sprite_2d_track_flip_set_process_x(ui_sprite_2d_track_flip_t track_flip, uint8_t process_x) {
    track_flip->m_process_x = process_x;
}

uint8_t ui_sprite_2d_track_flip_process_y(ui_sprite_2d_track_flip_t track_flip) {
    return track_flip->m_process_y;
}

void ui_sprite_2d_track_flip_set_process_y(ui_sprite_2d_track_flip_t track_flip, uint8_t process_y) {
    track_flip->m_process_y = process_y;
}

int ui_sprite_2d_track_flip_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_track_flip_t track_flip = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): track flip: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    ui_sprite_fsm_action_start_update(fsm_action);

    track_flip->m_pre_pos = ui_sprite_2d_transform_world_pos(transform, UI_SPRITE_2D_TRANSFORM_POS_ORIGIN, 0);

    return 0;
}

void ui_sprite_2d_track_flip_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_track_flip_t track_flip = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
	ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
    ui_vector_2 cur_pos;
    uint8_t flip_x;
    uint8_t flip_y;

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): track flip: update: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    cur_pos = ui_sprite_2d_transform_world_pos(transform, UI_SPRITE_2D_TRANSFORM_POS_ORIGIN, 0);

    flip_x = transform->m_data.transform.flip_x;
    flip_y = transform->m_data.transform.flip_y;

    if (track_flip->m_process_x && fabs(cur_pos.x - track_flip->m_pre_pos.x) > 0.5f) {
        flip_x = cur_pos.x > track_flip->m_pre_pos.x ? 0 : 1;
    }

    if (track_flip->m_process_y && fabs(cur_pos.y - track_flip->m_pre_pos.y) > 0.5f) {
        flip_y = cur_pos.y > track_flip->m_pre_pos.y ? 0 : 1;
    }

    ui_sprite_2d_transform_set_flip(transform, flip_x, flip_y);

    track_flip->m_pre_pos = cur_pos;
}

void ui_sprite_2d_track_flip_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

int ui_sprite_2d_track_flip_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_2d_track_flip_t track_flip = ui_sprite_fsm_action_data(fsm_action);

	bzero(track_flip, sizeof(*track_flip));
	track_flip->m_module = ctx;

	return 0;
}

int ui_sprite_2d_track_flip_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_2d_track_flip_t to_track_flip_to = ui_sprite_fsm_action_data(to);
    ui_sprite_2d_track_flip_t from_track_flip_to = ui_sprite_fsm_action_data(from);

    ui_sprite_2d_track_flip_init(to, ctx);

    to_track_flip_to->m_process_x = from_track_flip_to->m_process_x;
    to_track_flip_to->m_process_y = from_track_flip_to->m_process_y;

    return 0;
}

void ui_sprite_2d_track_flip_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

ui_sprite_fsm_action_t ui_sprite_2d_track_flip_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_track_flip_t p2d_track_flip = ui_sprite_2d_track_flip_create(fsm_state, name);

	if (p2d_track_flip == NULL) {
		CPE_ERROR(module->m_em, "%s: create anim_2d_track_flip action: create fail!", ui_sprite_2d_module_name(module));
		return NULL;
	}

    ui_sprite_2d_track_flip_set_process_x(
        p2d_track_flip,
        cfg_get_uint8(cfg, "process-x", ui_sprite_2d_track_flip_process_x(p2d_track_flip)));

    ui_sprite_2d_track_flip_set_process_y(
        p2d_track_flip,
        cfg_get_uint8(cfg, "process-y", ui_sprite_2d_track_flip_process_y(p2d_track_flip)));

	return ui_sprite_fsm_action_from_data(p2d_track_flip);
}


int ui_sprite_2d_track_flip_regist(ui_sprite_2d_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_2D_TRACK_FLIP_NAME, sizeof(struct ui_sprite_2d_track_flip));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_TRACK_FLIP_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_2d_track_flip_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_2d_track_flip_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_2d_track_flip_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_2d_track_flip_exit, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_2d_track_flip_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_2d_track_flip_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_2D_TRACK_FLIP_NAME, ui_sprite_2d_track_flip_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_2d_track_flip_unregist(ui_sprite_2d_module_t module) {
	ui_sprite_fsm_action_meta_t meta;

	meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_2D_TRACK_FLIP_NAME);
	if (meta == NULL) {
		CPE_ERROR(
			module->m_em, "%s: %s unregister: meta not exist",
			ui_sprite_2d_module_name(module), UI_SPRITE_2D_TRACK_FLIP_NAME);
		return;
	}

	ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_2D_TRACK_FLIP_NAME);
    }
}

const char * UI_SPRITE_2D_TRACK_FLIP_NAME = "2d-track-flip";
