#include <math.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_wait_stop.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_2d_wait_stop_i.h"
#include "ui_sprite_2d_module_i.h"
#include "protocol/ui/sprite_2d/ui_sprite_2d_evt.h"

ui_sprite_2d_wait_stop_t
ui_sprite_2d_wait_stop_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_2D_WAIT_STOP_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_2d_wait_stop_free(ui_sprite_2d_wait_stop_t show_anim) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_anim));
}

float ui_sprite_2d_wait_stop_threshold(ui_sprite_2d_wait_stop_t wait_stop) {
    return wait_stop->m_threshold;
}

void ui_sprite_2d_wait_stop_set_threshold(ui_sprite_2d_wait_stop_t wait_stop, float threshold) {
    wait_stop->m_threshold = threshold;
}

int ui_sprite_2d_wait_stop_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_wait_stop_t wait_stop = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): wait stop: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    wait_stop->m_stoped_time = 0.0f;
    wait_stop->m_pre_pos = ui_sprite_2d_transform_world_pos(transform, UI_SPRITE_2D_TRANSFORM_POS_ORIGIN, 0);

    ui_sprite_fsm_action_start_update(fsm_action);
    
    return 0;
}

void ui_sprite_2d_wait_stop_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_wait_stop_t wait_stop = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
	ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
    ui_vector_2 cur_pos;

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): wait stop: update: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    cur_pos = ui_sprite_2d_transform_origin_pos(transform);

    if (cur_pos.x != wait_stop->m_pre_pos.x || cur_pos.y != wait_stop->m_pre_pos.y) {
        wait_stop->m_stoped_time = 0.0f;
        wait_stop->m_pre_pos = cur_pos;
    }
    else {
        wait_stop->m_stoped_time += delta;
        if (wait_stop->m_stoped_time >= wait_stop->m_threshold) {
            ui_sprite_fsm_action_stop_update(fsm_action);            
        }
    }
}

void ui_sprite_2d_wait_stop_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

int ui_sprite_2d_wait_stop_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_2d_wait_stop_t wait_stop = ui_sprite_fsm_action_data(fsm_action);

	bzero(wait_stop, sizeof(*wait_stop));
	wait_stop->m_module = ctx;
    wait_stop->m_threshold = 0.0f;
    wait_stop->m_stoped_time = 0.0f;
    wait_stop->m_pre_pos.x = wait_stop->m_pre_pos.y = 0.0f;

	return 0;
}

int ui_sprite_2d_wait_stop_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_2d_wait_stop_t to_wait_stop_to = ui_sprite_fsm_action_data(to);
    ui_sprite_2d_wait_stop_t from_wait_stop_to = ui_sprite_fsm_action_data(from);

    ui_sprite_2d_wait_stop_init(to, ctx);

    to_wait_stop_to->m_threshold = from_wait_stop_to->m_threshold;

    return 0;
}

void ui_sprite_2d_wait_stop_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

ui_sprite_fsm_action_t ui_sprite_2d_wait_stop_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_wait_stop_t p2d_wait_stop = ui_sprite_2d_wait_stop_create(fsm_state, name);

	if (p2d_wait_stop == NULL) {
		CPE_ERROR(module->m_em, "%s: create anim_2d_wait_stop action: create fail!", ui_sprite_2d_module_name(module));
		return NULL;
	}

    ui_sprite_2d_wait_stop_set_threshold(
        p2d_wait_stop,
        cfg_get_float(cfg, "threshold", ui_sprite_2d_wait_stop_threshold(p2d_wait_stop)));

	return ui_sprite_fsm_action_from_data(p2d_wait_stop);
}

int ui_sprite_2d_wait_stop_regist(ui_sprite_2d_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_2D_WAIT_STOP_NAME, sizeof(struct ui_sprite_2d_wait_stop));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_WAIT_STOP_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_2d_wait_stop_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_2d_wait_stop_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_2d_wait_stop_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_2d_wait_stop_exit, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_2d_wait_stop_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_2d_wait_stop_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_2D_WAIT_STOP_NAME, ui_sprite_2d_wait_stop_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_2d_wait_stop_unregist(ui_sprite_2d_module_t module) {
	ui_sprite_fsm_action_meta_t meta;

	meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_2D_WAIT_STOP_NAME);
	if (meta == NULL) {
		CPE_ERROR(
			module->m_em, "%s: %s unregister: meta not exist",
			ui_sprite_2d_module_name(module), UI_SPRITE_2D_WAIT_STOP_NAME);
		return;
	}

	ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_2D_WAIT_STOP_NAME);
    }
}

const char * UI_SPRITE_2D_WAIT_STOP_NAME = "2d-wait-stop";
