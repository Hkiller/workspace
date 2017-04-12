#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_2d_wait_move_distance_i.h"

ui_sprite_2d_wait_move_distance_t
ui_sprite_2d_wait_move_distance_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_2D_WAIT_MOVE_DISTANCE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_2d_wait_move_distance_free(ui_sprite_2d_wait_move_distance_t show_anim) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_anim));
}

int ui_sprite_2d_wait_move_distance_set_distance(ui_sprite_2d_wait_move_distance_t wait_move_distance, const char * distance) {
    ui_sprite_2d_module_t module = wait_move_distance->m_module;
    
    if (wait_move_distance->m_cfg_distance) {
        mem_free(module->m_alloc, wait_move_distance->m_cfg_distance);
    }

    if (distance) {
        wait_move_distance->m_cfg_distance = cpe_str_mem_dup(module->m_alloc, distance);
        return wait_move_distance->m_cfg_distance == NULL ? -1 : 0;
    }
    else {
        wait_move_distance->m_cfg_distance = NULL;
        return 0;
    }
}

const char * ui_sprite_2d_wait_move_distance_distance(ui_sprite_2d_wait_move_distance_t wait_move_distance) {
    return wait_move_distance->m_cfg_distance;
}

int ui_sprite_2d_wait_move_distance_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_wait_move_distance_t wait_move_distance = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_2d_transform_t transform;

    if (wait_move_distance->m_cfg_distance == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): wait move distance: distance not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (ui_sprite_fsm_action_check_calc_float(
            &wait_move_distance->m_distance, wait_move_distance->m_cfg_distance, fsm_action, NULL, module->m_em) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): %s: wait move distance: calc distance from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_action_name(fsm_action),
            wait_move_distance->m_cfg_distance);
        return -1;
    }

    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): wait move distance: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    wait_move_distance->m_pre_pos = ui_sprite_2d_transform_world_pos(transform, UI_SPRITE_2D_TRANSFORM_POS_ORIGIN, 0);
    wait_move_distance->m_moved_distance = 0.0f;
    
    ui_sprite_fsm_action_start_update(fsm_action);

    return 0;
}

void ui_sprite_2d_wait_move_distance_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_wait_move_distance_t wait_move_distance = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
	ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
    ui_vector_2 cur_pos;
    float distance;
    
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): wait move distance: update: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    cur_pos = ui_sprite_2d_transform_origin_pos(transform);

    distance = cpe_math_distance(wait_move_distance->m_pre_pos.x, wait_move_distance->m_pre_pos.y, cur_pos.x, cur_pos.y);
    wait_move_distance->m_pre_pos = cur_pos;

    wait_move_distance->m_moved_distance += distance;

    if (wait_move_distance->m_moved_distance > wait_move_distance->m_distance) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

void ui_sprite_2d_wait_move_distance_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

int ui_sprite_2d_wait_move_distance_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_2d_wait_move_distance_t wait_move_distance = ui_sprite_fsm_action_data(fsm_action);

	bzero(wait_move_distance, sizeof(*wait_move_distance));
    
	wait_move_distance->m_module = ctx;

	return 0;
}

int ui_sprite_2d_wait_move_distance_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
    ui_sprite_2d_wait_move_distance_t to_wait_move_distance = ui_sprite_fsm_action_data(to);
    ui_sprite_2d_wait_move_distance_t from_wait_move_distance = ui_sprite_fsm_action_data(from);

    ui_sprite_2d_wait_move_distance_init(to, ctx);

    if (from_wait_move_distance->m_cfg_distance) {
        to_wait_move_distance->m_cfg_distance = cpe_str_mem_dup(module->m_alloc, from_wait_move_distance->m_cfg_distance);
    }

    return 0;
}

void ui_sprite_2d_wait_move_distance_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
    ui_sprite_2d_wait_move_distance_t wait_move_distance = ui_sprite_fsm_action_data(fsm_action);

    if (wait_move_distance->m_cfg_distance) {
        mem_free(module->m_alloc, wait_move_distance->m_cfg_distance);
        wait_move_distance->m_cfg_distance = NULL;
    }
}

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_2d_wait_move_distance(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_wait_move_distance_t p2d_wait_move_distance = ui_sprite_2d_wait_move_distance_create(fsm_state, name);
    const char * str_value;

	if (p2d_wait_move_distance == NULL) {
		CPE_ERROR(module->m_em, "%s: create anim_2d_wait_move_distance action: create fail!", ui_sprite_2d_module_name(module));
		return NULL;
	}

    if ((str_value = cfg_get_string(cfg, "distance", NULL))) {
        ui_sprite_2d_wait_move_distance_set_distance(p2d_wait_move_distance, str_value);
    }

	return ui_sprite_fsm_action_from_data(p2d_wait_move_distance);
}

ui_sprite_fsm_action_t ui_sprite_2d_wait_move_distance_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_wait_move_distance_t p2d_wait_move_distance = ui_sprite_2d_wait_move_distance_create(fsm_state, name);
    const char * str_value;

	if (p2d_wait_move_distance == NULL) {
		CPE_ERROR(module->m_em, "%s: create anim_2d_wait_move_distance action: create fail!", ui_sprite_2d_module_name(module));
		return NULL;
	}

    if ((str_value = cfg_get_string(cfg, "distance", NULL))) {
        ui_sprite_2d_wait_move_distance_set_distance(p2d_wait_move_distance, str_value);
    }

	return ui_sprite_fsm_action_from_data(p2d_wait_move_distance);
}

int ui_sprite_2d_wait_move_distance_regist(ui_sprite_2d_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_2D_WAIT_MOVE_DISTANCE_NAME, sizeof(struct ui_sprite_2d_wait_move_distance));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_WAIT_MOVE_DISTANCE_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_2d_wait_move_distance_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_2d_wait_move_distance_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_2d_wait_move_distance_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_2d_wait_move_distance_exit, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_2d_wait_move_distance_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_2d_wait_move_distance_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_2D_WAIT_MOVE_DISTANCE_NAME, ui_sprite_2d_wait_move_distance_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_2d_wait_move_distance_unregist(ui_sprite_2d_module_t module) {
	ui_sprite_fsm_action_meta_t meta;

	meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_2D_WAIT_MOVE_DISTANCE_NAME);
	if (meta == NULL) {
		CPE_ERROR(
			module->m_em, "%s: %s unregister: meta not exist",
			ui_sprite_2d_module_name(module), UI_SPRITE_2D_WAIT_MOVE_DISTANCE_NAME);
		return;
	}

	ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_2D_WAIT_MOVE_DISTANCE_NAME);
    }
}

const char * UI_SPRITE_2D_WAIT_MOVE_DISTANCE_NAME = "2d-wait-move-distance";
