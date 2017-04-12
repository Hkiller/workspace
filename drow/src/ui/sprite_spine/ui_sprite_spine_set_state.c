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
#include "ui_sprite_spine_set_state_i.h"
#include "plugin/spine/plugin_spine_types.h"
#include "plugin/spine/plugin_spine_obj_part.h"

ui_sprite_spine_set_state_t ui_sprite_spine_set_state_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_SPINE_SET_STATE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_spine_set_state_free(ui_sprite_spine_set_state_t set_state) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(set_state);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_spine_set_state_set_part(ui_sprite_spine_set_state_t action_set_state, const char * part) {
    assert(part);

    if (action_set_state->m_cfg_part) {
        mem_free(action_set_state->m_module->m_alloc, action_set_state->m_cfg_part);
        action_set_state->m_cfg_part = NULL;
    }

    action_set_state->m_cfg_part = cpe_str_mem_dup_trim(action_set_state->m_module->m_alloc, part);
    
    return 0;
}

int ui_sprite_spine_set_state_set_state(ui_sprite_spine_set_state_t action_set_state, const char * state) {
    assert(state);

    if (action_set_state->m_cfg_state) {
        mem_free(action_set_state->m_module->m_alloc, action_set_state->m_cfg_state);
        action_set_state->m_cfg_state = NULL;
    }

    action_set_state->m_cfg_state = cpe_str_mem_dup_trim(action_set_state->m_module->m_alloc, state);
    
    return 0;
}


static int ui_sprite_spine_set_state_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_set_state_t set_state = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    char * part_name = NULL;
    plugin_spine_obj_part_t obj_part;

    part_name =
        ui_sprite_fsm_action_check_calc_str_dup(module->m_alloc, set_state->m_cfg_part, fsm_action, NULL, module->m_em);
    if (part_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_sprite_spine_set_state: calc part %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            set_state->m_cfg_part);
        goto ENTER_FAIL;
    }

    obj_part = ui_sprite_spine_find_obj_part(module->m_sprite_render, entity, part_name, module->m_em);
    if (obj_part == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_sprite_spine_set_state: part %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            part_name);
        goto ENTER_FAIL;
    }

    if (plugin_spine_obj_part_set_cur_state_by_name(obj_part, set_state->m_cfg_state) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_sprite_spine_set_state: %s apply state %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            part_name, set_state->m_cfg_state);
        goto ENTER_FAIL;
    }

    mem_free(module->m_alloc, part_name);

    return 0;

ENTER_FAIL:
    if (part_name) {
        mem_free(module->m_alloc, part_name);
    }

    return -1; 
}

static void ui_sprite_spine_set_state_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_spine_set_state_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_set_state_t set_state = ui_sprite_fsm_action_data(fsm_action);
	bzero(set_state, sizeof(*set_state));
    set_state->m_module = ctx;
    set_state->m_cfg_part = NULL;
    set_state->m_cfg_state = NULL;
    return 0;
}

static void ui_sprite_spine_set_state_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t modue = ctx;
    ui_sprite_spine_set_state_t action_set_state = ui_sprite_fsm_action_data(fsm_action);

    if (action_set_state->m_cfg_part) {
        mem_free(modue->m_alloc, action_set_state->m_cfg_part);
        action_set_state->m_cfg_part = NULL;
    }

    if (action_set_state->m_cfg_state) {
        mem_free(modue->m_alloc, action_set_state->m_cfg_state);
        action_set_state->m_cfg_state = NULL;
    }
}

static int ui_sprite_spine_set_state_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_spine_module_t modue = ctx;
    ui_sprite_spine_set_state_t to_set_state = ui_sprite_fsm_action_data(to);
    ui_sprite_spine_set_state_t from_set_state = ui_sprite_fsm_action_data(from);

    if (ui_sprite_spine_set_state_init(to, ctx)) return -1;

    if (from_set_state->m_cfg_part) {
        to_set_state->m_cfg_part = cpe_str_mem_dup(modue->m_alloc, from_set_state->m_cfg_part);
    }
    
    if (from_set_state->m_cfg_state) {
        to_set_state->m_cfg_state = cpe_str_mem_dup(modue->m_alloc, from_set_state->m_cfg_state);
    }

    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_spine_set_state_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_set_state_t spine_set_state = ui_sprite_spine_set_state_create(fsm_state, name);
    const char * str_value;

    if (spine_set_state == NULL) {
        CPE_ERROR(module->m_em, "%s: create spine_set_state action: create fail!", ui_sprite_spine_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "part", NULL))) {
        if (ui_sprite_spine_set_state_set_part(spine_set_state, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create spine_set_state action: set obj name %s fail!",
                ui_sprite_spine_module_name(module), str_value);
            ui_sprite_spine_set_state_free(spine_set_state);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create spine_set_state action: part not configured!", ui_sprite_spine_module_name(module));
        ui_sprite_spine_set_state_free(spine_set_state);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "state", NULL))) {
        if (ui_sprite_spine_set_state_set_state(spine_set_state, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create spine_set_state action: set obj name %s fail!",
                ui_sprite_spine_module_name(module), str_value);
            ui_sprite_spine_set_state_free(spine_set_state);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create spine_set_state action: state not configured!", ui_sprite_spine_module_name(module));
        ui_sprite_spine_set_state_free(spine_set_state);
        return NULL;
    }
    
    return ui_sprite_fsm_action_from_data(spine_set_state);
}

int ui_sprite_spine_set_state_regist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_SPINE_SET_STATE_NAME, sizeof(struct ui_sprite_spine_set_state));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_spine_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_spine_set_state_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_spine_set_state_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_spine_set_state_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_spine_set_state_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_spine_set_state_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_SPINE_SET_STATE_NAME, ui_sprite_spine_set_state_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_spine_set_state_unregist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_SPINE_SET_STATE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_spine_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_SPINE_SET_STATE_NAME = "spine-set-state";

