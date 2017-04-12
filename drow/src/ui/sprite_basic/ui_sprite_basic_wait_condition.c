#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_basic_wait_condition_i.h"

ui_sprite_basic_wait_condition_t ui_sprite_basic_wait_condition_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_BASIC_WAIT_CONDITION_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_basic_wait_condition_free(ui_sprite_basic_wait_condition_t wait_condition) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(wait_condition);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_basic_wait_condition_set_check(ui_sprite_basic_wait_condition_t wait_condition, const char * condition) {
    ui_sprite_basic_module_t module = wait_condition->m_module;

    if (wait_condition->m_cfg_check) mem_free(module->m_alloc, wait_condition->m_cfg_check);

    if (condition) {
        wait_condition->m_cfg_check = cpe_str_mem_dup_trim(module->m_alloc, condition);
        return wait_condition->m_cfg_check == NULL ? -1 : 0;
    }
    else {
        wait_condition->m_cfg_check = NULL;
        return 0;
    }
}

const char * ui_sprite_basic_wait_condition_check(ui_sprite_basic_wait_condition_t wait_condition) {
    return wait_condition->m_cfg_check;
}

static int ui_sprite_basic_wait_condition_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_wait_condition_t wait_condition = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (wait_condition->m_cfg_check == NULL) {
        CPE_ERROR(
            wait_condition->m_module->m_em, "entity %d(%s): %s: enter: no check!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), UI_SPRITE_BASIC_WAIT_CONDITION_NAME);
        return -1;
    }
    
    ui_sprite_fsm_action_start_update(fsm_action);

    return 0;
}

static void ui_sprite_basic_wait_condition_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_wait_condition_t wait_condition = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    
    uint8_t result;
    if (ui_sprite_fsm_action_check_calc_bool(&result, wait_condition->m_cfg_check, fsm_action, NULL, module->m_em) != 0) {
        CPE_ERROR(
            wait_condition->m_module->m_em, "entity %d(%s): %s: enter: calc check from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), UI_SPRITE_BASIC_WAIT_CONDITION_NAME, wait_condition->m_cfg_check);
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    if (result) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static void ui_sprite_basic_wait_condition_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_basic_wait_condition_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_wait_condition_t wait_condition = ui_sprite_fsm_action_data(fsm_action);
    wait_condition->m_module = ctx;
	wait_condition->m_cfg_check = NULL;
    return 0;
}

static void ui_sprite_basic_wait_condition_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_wait_condition_t wait_condition = ui_sprite_fsm_action_data(fsm_action);

    if (wait_condition->m_cfg_check) {
        mem_free(module->m_alloc, wait_condition->m_cfg_check);
        wait_condition->m_cfg_check = NULL;
    }
}

static int ui_sprite_basic_wait_condition_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_wait_condition_t to_wait_condition = ui_sprite_fsm_action_data(to);
    ui_sprite_basic_wait_condition_t from_wait_condition = ui_sprite_fsm_action_data(from);

    if (ui_sprite_basic_wait_condition_init(to, ctx)) return -1;

    if (from_wait_condition->m_cfg_check) {
        to_wait_condition->m_cfg_check = cpe_str_mem_dup(module->m_alloc, from_wait_condition->m_cfg_check);
    }

    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_basic_wait_condition_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_wait_condition_t wait_condition = ui_sprite_basic_wait_condition_create(fsm_state, name);

    if (wait_condition == NULL) {
        CPE_ERROR(module->m_em, "%s: create wait_condition action: create fail!", ui_sprite_basic_module_name(module));
        return NULL;
    }

    if (ui_sprite_basic_wait_condition_set_check(wait_condition, cfg_get_string(cfg, "check", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create %s: set condition fail",
            ui_sprite_basic_module_name(module), UI_SPRITE_BASIC_WAIT_CONDITION_NAME);
        ui_sprite_basic_wait_condition_free(wait_condition);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(wait_condition);
}

int ui_sprite_basic_wait_condition_regist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_BASIC_WAIT_CONDITION_NAME, sizeof(struct ui_sprite_basic_wait_condition));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim wait condition register: meta create fail",
            ui_sprite_basic_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_basic_wait_condition_enter, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_basic_wait_condition_update, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_basic_wait_condition_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_basic_wait_condition_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_basic_wait_condition_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_basic_wait_condition_clear, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_BASIC_WAIT_CONDITION_NAME, ui_sprite_basic_wait_condition_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_basic_wait_condition_unregist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_BASIC_WAIT_CONDITION_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: wait condition unregister: meta not exist",
            ui_sprite_basic_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_BASIC_WAIT_CONDITION_NAME);
    }
}

const char * UI_SPRITE_BASIC_WAIT_CONDITION_NAME = "wait-condition";

