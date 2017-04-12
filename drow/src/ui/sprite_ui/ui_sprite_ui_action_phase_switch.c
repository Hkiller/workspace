#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_control.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_module.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_ui_action_phase_switch_i.h"
#include "ui_sprite_ui_env_i.h"

ui_sprite_ui_action_phase_switch_t
ui_sprite_ui_action_phase_switch_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_UI_ACTION_PHASE_SWITCH_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ui_action_phase_switch_free(ui_sprite_ui_action_phase_switch_t show_ui) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_ui));
}

const char * ui_sprite_ui_action_phase_switch_to(ui_sprite_ui_action_phase_switch_t phase_switch) {
    return phase_switch->m_cfg_to;
}

void ui_sprite_ui_action_phase_switch_set_to(ui_sprite_ui_action_phase_switch_t phase_switch, const char * to) {
    if (phase_switch->m_cfg_to) {
        mem_free(phase_switch->m_module->m_alloc, phase_switch->m_cfg_to);
    }

    if (to) {
        phase_switch->m_cfg_to = cpe_str_mem_dup_trim(phase_switch->m_module->m_alloc, to);
    }
    else {
        phase_switch->m_cfg_to = NULL;
    }
}

const char * ui_sprite_ui_action_phase_switch_loading(ui_sprite_ui_action_phase_switch_t phase_switch) {
    return phase_switch->m_cfg_loading;
}

void ui_sprite_ui_action_phase_switch_set_loading(ui_sprite_ui_action_phase_switch_t phase_switch, const char * loading) {
    if (phase_switch->m_cfg_loading) {
        mem_free(phase_switch->m_module->m_alloc, phase_switch->m_cfg_loading);
    }

    if (loading) {
        phase_switch->m_cfg_loading = cpe_str_mem_dup_trim(phase_switch->m_module->m_alloc, loading);
    }
    else {
        phase_switch->m_cfg_loading = NULL;
    }
}


static int ui_sprite_ui_action_phase_switch_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_phase_switch_t phase_switch = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_ui_env_t env = ui_sprite_ui_env_find(world);

    if (phase_switch->m_cfg_to == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): phase switch: switch to not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    
    if (plugin_ui_env_phase_switch_by_name(env->m_env, phase_switch->m_cfg_to, phase_switch->m_cfg_loading, NULL) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): phase switch: switch to %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), phase_switch->m_cfg_to);
        return -1;
    }
    
    ui_sprite_fsm_action_start_update(fsm_action);

    return 0;
}

static void ui_sprite_ui_action_phase_switch_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static void ui_sprite_ui_action_phase_switch_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
}

static int ui_sprite_ui_action_phase_switch_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_action_phase_switch_t phase_switch = ui_sprite_fsm_action_data(fsm_action);

    phase_switch->m_module = ctx;
    phase_switch->m_cfg_to = NULL;
    phase_switch->m_cfg_loading = NULL;
    
    return 0;
}

static int ui_sprite_ui_action_phase_switch_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_phase_switch_t to_phase_switch = ui_sprite_fsm_action_data(to);
    ui_sprite_ui_action_phase_switch_t from_phase_switch = ui_sprite_fsm_action_data(from);
    
    ui_sprite_ui_action_phase_switch_init(to, ctx);

    if (from_phase_switch->m_cfg_to) {
        to_phase_switch->m_cfg_to = cpe_str_mem_dup(module->m_alloc, from_phase_switch->m_cfg_to);
    }

    if (from_phase_switch->m_cfg_loading) {
        to_phase_switch->m_cfg_loading = cpe_str_mem_dup(module->m_alloc, from_phase_switch->m_cfg_loading);
    }
    
    return 0;
}

static void ui_sprite_ui_action_phase_switch_fini(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_phase_switch_t phase_switch = ui_sprite_fsm_action_data(fsm_action);
    
    if (phase_switch->m_cfg_to) {
        mem_free(module->m_alloc, phase_switch->m_cfg_to);
        phase_switch->m_cfg_to = NULL;
    }

    if (phase_switch->m_cfg_loading) {
        mem_free(module->m_alloc, phase_switch->m_cfg_loading);
        phase_switch->m_cfg_loading = NULL;
    }
}

static ui_sprite_fsm_action_t
ui_sprite_ui_action_phase_switch_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_phase_switch_t phase_switch = ui_sprite_ui_action_phase_switch_create(fsm_state, name);
    const char * str_value;

    if (phase_switch == NULL) {
        CPE_ERROR(module->m_em, "%s: create phase_switch action: create fail!", ui_sprite_ui_module_name(module));
        return NULL;
    }

    str_value = cfg_get_string(cfg, "to", NULL);
    if (str_value == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create ui-phase-switch: to not configured!",
            ui_sprite_ui_module_name(module));
        ui_sprite_ui_action_phase_switch_free(phase_switch);
        return NULL;
    }
    else {
        ui_sprite_ui_action_phase_switch_set_to(phase_switch, str_value);
    }

    if ((str_value = cfg_get_string(cfg, "loading", NULL))) {
        ui_sprite_ui_action_phase_switch_set_loading(phase_switch, str_value);
    }

    return ui_sprite_fsm_action_from_data(phase_switch);
}

int ui_sprite_ui_action_phase_switch_regist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_UI_ACTION_PHASE_SWITCH_NAME, sizeof(struct ui_sprite_ui_action_phase_switch));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_ui_module_name(module), UI_SPRITE_UI_ACTION_PHASE_SWITCH_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ui_action_phase_switch_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ui_action_phase_switch_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ui_action_phase_switch_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ui_action_phase_switch_exit, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_ui_action_phase_switch_update, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ui_action_phase_switch_fini, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(
                module->m_loader,
                UI_SPRITE_UI_ACTION_PHASE_SWITCH_NAME,
                ui_sprite_ui_action_phase_switch_load, module) != 0)
        {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_ui_action_phase_switch_unregist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_UI_ACTION_PHASE_SWITCH_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_ui_module_name(module), UI_SPRITE_UI_ACTION_PHASE_SWITCH_NAME);
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_UI_ACTION_PHASE_SWITCH_NAME);
    }
}

const char * UI_SPRITE_UI_ACTION_PHASE_SWITCH_NAME = "ui-phase-switch";
