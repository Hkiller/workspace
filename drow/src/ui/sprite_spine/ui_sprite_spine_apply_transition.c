#include <assert.h>
#include "gd/app/app_log.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/pal/pal_strings.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_spine/ui_sprite_spine_utils.h"
#include "ui_sprite_spine_apply_transition_i.h"
#include "plugin/spine/plugin_spine_types.h"
#include "plugin/spine/plugin_spine_obj_part.h"

ui_sprite_spine_apply_transition_t ui_sprite_spine_apply_transition_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_SPINE_APPLY_TRANSITION_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_spine_apply_transition_free(ui_sprite_spine_apply_transition_t apply_transition) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(apply_transition);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_spine_apply_transition_set_part(ui_sprite_spine_apply_transition_t action_apply_transition, const char * part) {
    assert(part);

    if (action_apply_transition->m_cfg_part) {
        mem_free(action_apply_transition->m_module->m_alloc, action_apply_transition->m_cfg_part);
        action_apply_transition->m_cfg_part = NULL;
    }

    action_apply_transition->m_cfg_part = cpe_str_mem_dup_trim(action_apply_transition->m_module->m_alloc, part);
    
    return 0;
}

int ui_sprite_spine_apply_transition_set_transition(ui_sprite_spine_apply_transition_t action_apply_transition, const char * transition) {
    assert(transition);

    if (action_apply_transition->m_cfg_transition) {
        mem_free(action_apply_transition->m_module->m_alloc, action_apply_transition->m_cfg_transition);
        action_apply_transition->m_cfg_transition = NULL;
    }

    action_apply_transition->m_cfg_transition = cpe_str_mem_dup_trim(action_apply_transition->m_module->m_alloc, transition);
    
    return 0;
}


static int ui_sprite_spine_apply_transition_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_apply_transition_t apply_transition = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    plugin_spine_obj_part_t obj_part;
    char * transition = NULL;;
    
    assert(apply_transition->m_part == NULL);

    apply_transition->m_part =
        ui_sprite_fsm_action_check_calc_str_dup(module->m_alloc, apply_transition->m_cfg_part, fsm_action, NULL, module->m_em);
    if (apply_transition->m_part == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_sprite_spine_apply_transition: calc part %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            apply_transition->m_cfg_part);
        goto ENTER_FAIL;
    }

    obj_part = ui_sprite_spine_find_obj_part(module->m_sprite_render, entity, apply_transition->m_part, module->m_em);
    if (obj_part == NULL) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): ui_sprite_spine_apply_transition: part %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                apply_transition->m_part);
        }
        return 0;
    }

    transition = ui_sprite_fsm_action_check_calc_str_dup(module->m_alloc, apply_transition->m_cfg_transition, fsm_action, NULL, module->m_em);
    if (transition == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_sprite_spine_apply_transition: calc transition %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            apply_transition->m_cfg_transition);
        goto ENTER_FAIL;
    }

    
    if (plugin_spine_obj_part_apply_transition_by_name(obj_part, transition) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_sprite_spine_apply_transition: %s apply transition %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            apply_transition->m_part, transition);
        goto ENTER_FAIL;
    }

    if (ui_sprite_fsm_action_life_circle(fsm_action) == ui_sprite_fsm_action_life_circle_working) {
        ui_sprite_fsm_action_start_update(fsm_action);
    }

    mem_free(module->m_alloc, transition);
    
    return 0;

ENTER_FAIL:
    if (apply_transition->m_part) {
        mem_free(module->m_alloc, apply_transition->m_part);
        apply_transition->m_part = NULL;
    }

    if (transition) {
        mem_free(module->m_alloc, transition);
    }

    return -1; 
}

static void ui_sprite_spine_apply_transition_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_apply_transition_t apply_transition = ui_sprite_fsm_action_data(fsm_action);

    assert(apply_transition->m_part);
    mem_free(module->m_alloc, apply_transition->m_part);
    apply_transition->m_part = NULL;
}

void ui_sprite_spine_apply_transition_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_apply_transition_t apply_transition = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    plugin_spine_obj_part_t obj_part;

    assert(apply_transition->m_part);
    
    obj_part = ui_sprite_spine_find_obj_part(module->m_sprite_render, entity, apply_transition->m_part, module->m_em);
    if (obj_part == NULL) {
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    if (!plugin_spine_obj_part_is_in_enter(obj_part)) {
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }
}

static int ui_sprite_spine_apply_transition_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_apply_transition_t apply_transition = ui_sprite_fsm_action_data(fsm_action);
	bzero(apply_transition, sizeof(*apply_transition));
    apply_transition->m_module = ctx;
    apply_transition->m_cfg_part = NULL;
    apply_transition->m_cfg_transition = NULL;
    apply_transition->m_part = NULL;
    return 0;
}

static void ui_sprite_spine_apply_transition_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t modue = ctx;
    ui_sprite_spine_apply_transition_t action_apply_transition = ui_sprite_fsm_action_data(fsm_action);

    assert(action_apply_transition->m_part == NULL);
    
    if (action_apply_transition->m_cfg_part) {
        mem_free(modue->m_alloc, action_apply_transition->m_cfg_part);
        action_apply_transition->m_cfg_part = NULL;
    }

    if (action_apply_transition->m_cfg_transition) {
        mem_free(modue->m_alloc, action_apply_transition->m_cfg_transition);
        action_apply_transition->m_cfg_transition = NULL;
    }
}

static int ui_sprite_spine_apply_transition_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_spine_module_t modue = ctx;
    ui_sprite_spine_apply_transition_t to_apply_transition = ui_sprite_fsm_action_data(to);
    ui_sprite_spine_apply_transition_t from_apply_transition = ui_sprite_fsm_action_data(from);

    if (ui_sprite_spine_apply_transition_init(to, ctx)) return -1;

    if (from_apply_transition->m_cfg_part) {
        to_apply_transition->m_cfg_part = cpe_str_mem_dup(modue->m_alloc, from_apply_transition->m_cfg_part);
    }
    
    if (from_apply_transition->m_cfg_transition) {
        to_apply_transition->m_cfg_transition = cpe_str_mem_dup(modue->m_alloc, from_apply_transition->m_cfg_transition);
    }

    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_spine_apply_transition_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_apply_transition_t spine_apply_transition = ui_sprite_spine_apply_transition_create(fsm_state, name);
    const char * str_value;

    if (spine_apply_transition == NULL) {
        CPE_ERROR(module->m_em, "%s: spine-apply-transition: create fail!", ui_sprite_spine_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "part", NULL))) {
        if (ui_sprite_spine_apply_transition_set_part(spine_apply_transition, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: spine-apply-transition: set obj name %s fail!",
                ui_sprite_spine_module_name(module), str_value);
            ui_sprite_spine_apply_transition_free(spine_apply_transition);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: spine-apply-transition: part not configured!", ui_sprite_spine_module_name(module));
        ui_sprite_spine_apply_transition_free(spine_apply_transition);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "transition", NULL))) {
        if (ui_sprite_spine_apply_transition_set_transition(spine_apply_transition, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: spine-apply-transition: set transition %s fail!",
                ui_sprite_spine_module_name(module), str_value);
            ui_sprite_spine_apply_transition_free(spine_apply_transition);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: spine-apply-transition: transition not configured!", ui_sprite_spine_module_name(module));
        ui_sprite_spine_apply_transition_free(spine_apply_transition);
        return NULL;
    }
    
    return ui_sprite_fsm_action_from_data(spine_apply_transition);
}

int ui_sprite_spine_apply_transition_regist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_SPINE_APPLY_TRANSITION_NAME, sizeof(struct ui_sprite_spine_apply_transition));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_spine_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_spine_apply_transition_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_spine_apply_transition_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_spine_apply_transition_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_spine_apply_transition_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_spine_apply_transition_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_spine_apply_transition_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_SPINE_APPLY_TRANSITION_NAME, ui_sprite_spine_apply_transition_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_spine_apply_transition_unregist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_SPINE_APPLY_TRANSITION_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_spine_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_SPINE_APPLY_TRANSITION_NAME = "spine-apply-transition";

