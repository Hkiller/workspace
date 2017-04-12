#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/buffer.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_transform.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_2d/ui_sprite_2d_part.h"
#include "ui_sprite_2d_action_part_follow_i.h"
#include "ui_sprite_2d_module_i.h"

ui_sprite_2d_action_part_follow_t
ui_sprite_2d_action_part_follow_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_2D_ACTION_PART_FOLLOW_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_2d_action_part_follow_free(ui_sprite_2d_action_part_follow_t part_follow) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(part_follow));
}

int ui_sprite_2d_action_part_follow_set_part(ui_sprite_2d_action_part_follow_t part_follow, const char * part) {
    if (part_follow->m_cfg_part) {
        mem_free(part_follow->m_module->m_alloc, part_follow->m_cfg_part);
        part_follow->m_cfg_part = NULL;
    }

    if (part) {
        part_follow->m_cfg_part = cpe_str_mem_dup_trim(part_follow->m_module->m_alloc, part);
    }
    else {
        part_follow->m_cfg_part = NULL;
    }
    
    return 0;
}

int ui_sprite_2d_action_part_follow_set_target(ui_sprite_2d_action_part_follow_t part_follow, const char * target) {
    if (part_follow->m_cfg_target) {
        mem_free(part_follow->m_module->m_alloc, part_follow->m_cfg_target);
        part_follow->m_cfg_target = NULL;
    }

    if (target) {
        part_follow->m_cfg_target = cpe_str_mem_dup_trim(part_follow->m_module->m_alloc, target);
    }
    else {
        part_follow->m_cfg_target = NULL;
    }
    
    return 0;
}

int ui_sprite_2d_action_part_follow_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_action_part_follow_t part_follow = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
    struct mem_buffer buffer;
    const char * part_name;
    const char * target;
    const char * sep;
    
    mem_buffer_init(&buffer, module->m_alloc);
    
    assert(part_follow->m_part == NULL);
    assert(part_follow->m_target_entity == 0);
    assert(part_follow->m_target_part == NULL);
    
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): part follow: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    if (part_follow->m_cfg_part == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): part follow: part not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    if (part_follow->m_cfg_target == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): part follow: target not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }
    
    part_name = ui_sprite_fsm_action_check_calc_str(&buffer, part_follow->m_cfg_part, fsm_action, NULL, module->m_em);
    if (part_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): part follow: calc part from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), part_follow->m_cfg_part);
        goto ENTER_FAIL;
    }

    part_follow->m_part = ui_sprite_2d_part_find(transform, part_name);
    if (part_follow->m_part == NULL) {
        part_follow->m_part = ui_sprite_2d_part_create(transform, part_name);
        if (part_follow->m_part == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): part follow: calc part from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), part_follow->m_cfg_part);
            goto ENTER_FAIL;
        }
    }

    mem_buffer_clear_data(&buffer);
    target = ui_sprite_fsm_action_check_calc_str(&buffer, part_follow->m_cfg_target, fsm_action, NULL, module->m_em);
    if (part_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): part follow: calc target from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), part_follow->m_cfg_target);
        goto ENTER_FAIL;
    }


    sep = strchr(target, '.');
    if (sep == NULL) {
        part_follow->m_target_entity = cpe_str_mem_dup_trim(module->m_alloc, target);
        assert(part_follow->m_target_part == NULL);
    }
    else {
        part_follow->m_target_entity = cpe_str_mem_dup_range(module->m_alloc, target, sep);
        part_follow->m_target_part = cpe_str_mem_dup(module->m_alloc, sep + 1);
    }
    
    ui_sprite_fsm_action_start_update(fsm_action);
    
    return 0;

ENTER_FAIL:
    mem_buffer_clear(&buffer);
    
    part_follow->m_part = NULL;

    if (part_follow->m_target_entity) {
        mem_free(module->m_alloc, part_follow->m_target_entity);
        part_follow->m_target_entity = NULL;
    }
    
    if (part_follow->m_target_part) {
        mem_free(module->m_alloc, part_follow->m_target_part);
        part_follow->m_target_part = NULL;
    }
    
    return -1;
}

void ui_sprite_2d_action_part_follow_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_2d_module_t module = ctx;
    ui_sprite_2d_action_part_follow_t part_follow = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_transform trans;
    ui_sprite_entity_t target_entity;
    ui_sprite_2d_transform_t target_transform;
    
    assert(part_follow->m_part);
    assert(part_follow->m_target_entity);

    target_entity = ui_sprite_entity_find_auto_select(ui_sprite_entity_world(entity), part_follow->m_target_entity);
    if (target_entity == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): part follow: target entity %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), part_follow->m_target_entity);
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    target_transform = ui_sprite_2d_transform_find(target_entity);
    if (target_transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): part follow: target entity %d(%s) no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_entity_id(target_entity), ui_sprite_entity_name(target_entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }
    
    if (part_follow->m_target_part) {
        ui_sprite_2d_part_t target_part = ui_sprite_2d_part_find(target_transform, part_follow->m_target_part);
        if (target_part) {
            if (ui_sprite_2d_part_calc_world_trans(target_part, &trans) != 0) return;
        }
        else {
            if (ui_sprite_2d_transform_calc_trans(target_transform, &trans) != 0) return;
        }
    }
    else {
        if (ui_sprite_2d_transform_calc_trans(target_transform, &trans) != 0) return;
    }

    if (trans.m_s.x == 0.0f || trans.m_s.y == 0.0f || trans.m_s.z == 0.0f) return;

    ui_sprite_2d_part_set_world_trans(part_follow->m_part, &trans);
}

void ui_sprite_2d_action_part_follow_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
    ui_sprite_2d_action_part_follow_t part_follow = ui_sprite_fsm_action_data(fsm_action);

    assert(part_follow->m_part);
    part_follow->m_part = NULL;
    
    assert(part_follow->m_target_entity);
    mem_free(module->m_alloc, part_follow->m_target_entity);
    part_follow->m_target_entity = NULL;

    if (part_follow->m_target_part) {
        mem_free(module->m_alloc, part_follow->m_target_part);
        part_follow->m_target_part = NULL;
    }
}

int ui_sprite_2d_action_part_follow_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_2d_action_part_follow_t part_follow = ui_sprite_fsm_action_data(fsm_action);

	part_follow->m_module = ctx;
    part_follow->m_cfg_part = NULL;
    part_follow->m_cfg_target = NULL;

    part_follow->m_part = NULL;
    part_follow->m_target_entity = NULL;
    part_follow->m_target_part = NULL;

	return 0;
}

int ui_sprite_2d_action_part_follow_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
    ui_sprite_2d_action_part_follow_t to_action_part_follow = ui_sprite_fsm_action_data(to);
    ui_sprite_2d_action_part_follow_t from_action_part_follow = ui_sprite_fsm_action_data(from);

    ui_sprite_2d_action_part_follow_init(to, ctx);

    if (from_action_part_follow->m_cfg_part) {
        to_action_part_follow->m_cfg_part = cpe_str_mem_dup(module->m_alloc, from_action_part_follow->m_cfg_part);
    }

    if (from_action_part_follow->m_cfg_target) {
        to_action_part_follow->m_cfg_target = cpe_str_mem_dup(module->m_alloc, from_action_part_follow->m_cfg_target);
    }
    
    return 0;
}

void ui_sprite_2d_action_part_follow_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
    ui_sprite_2d_action_part_follow_t part_follow = ui_sprite_fsm_action_data(fsm_action);

    assert(part_follow->m_part == NULL);
    assert(part_follow->m_target_entity == NULL);
    assert(part_follow->m_target_part == NULL);

    if (part_follow->m_cfg_part) {
        mem_free(module->m_alloc, part_follow->m_cfg_part);
        part_follow->m_cfg_part = NULL;
    }

    if (part_follow->m_cfg_target) {
        mem_free(module->m_alloc, part_follow->m_cfg_target);
        part_follow->m_cfg_target = NULL;
    }
}

ui_sprite_fsm_action_t ui_sprite_2d_action_part_follow_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_action_part_follow_t part_follow = ui_sprite_2d_action_part_follow_create(fsm_state, name);
    const char * str_value;

	if (part_follow == NULL) {
		CPE_ERROR(module->m_em, "%s: create part follow action: create fail!", ui_sprite_2d_module_name(module));
		return NULL;
	}

    if ((str_value = cfg_get_string(cfg, "part", NULL))) {
        if (ui_sprite_2d_action_part_follow_set_part(part_follow, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create part follow action: set part name %s fail!",
                ui_sprite_2d_module_name(module), str_value);
            ui_sprite_2d_action_part_follow_free(part_follow);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create part follow action: part not configured!",
            ui_sprite_2d_module_name(module));
        ui_sprite_2d_action_part_follow_free(part_follow);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "target", NULL))) {
        if (ui_sprite_2d_action_part_follow_set_target(part_follow, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create part follow action: set target name %s fail!",
                ui_sprite_2d_module_name(module), str_value);
            ui_sprite_2d_action_part_follow_free(part_follow);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create part follow action: target not configured!",
            ui_sprite_2d_module_name(module));
        ui_sprite_2d_action_part_follow_free(part_follow);
        return NULL;
    }
    
	return ui_sprite_fsm_action_from_data(part_follow);
}

int ui_sprite_2d_action_part_follow_regist(ui_sprite_2d_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_2D_ACTION_PART_FOLLOW_NAME, sizeof(struct ui_sprite_2d_action_part_follow));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_ACTION_PART_FOLLOW_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_2d_action_part_follow_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_2d_action_part_follow_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_2d_action_part_follow_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_2d_action_part_follow_exit, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_2d_action_part_follow_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_2d_action_part_follow_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(
                module->m_loader, UI_SPRITE_2D_ACTION_PART_FOLLOW_NAME, ui_sprite_2d_action_part_follow_load, module)
            != 0)
        {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_2d_action_part_follow_unregist(ui_sprite_2d_module_t module) {
	ui_sprite_fsm_action_meta_t meta;

	meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_2D_ACTION_PART_FOLLOW_NAME);
	if (meta == NULL) {
		CPE_ERROR(
			module->m_em, "%s: %s unregister: meta not exist",
			ui_sprite_2d_module_name(module), UI_SPRITE_2D_ACTION_PART_FOLLOW_NAME);
		return;
	}

	ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_2D_ACTION_PART_FOLLOW_NAME);
    }
}

const char * UI_SPRITE_2D_ACTION_PART_FOLLOW_NAME = "part-follow";
