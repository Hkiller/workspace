#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin/ui/plugin_ui_navigation.h"
#include "plugin/ui/plugin_ui_phase_node.h"
#include "plugin/ui/plugin_ui_state_node.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_ui_action_navigation_i.h"
#include "ui_sprite_ui_env_i.h"

ui_sprite_ui_action_navigation_t ui_sprite_ui_action_navigation_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_UI_ACTION_NAVIGATION_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ui_action_navigation_free(ui_sprite_ui_action_navigation_t action_navigation) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(action_navigation);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_ui_action_navigation_set_event(ui_sprite_ui_action_navigation_t action_navigation, const char * event, const char * condition) {
    assert(event);

    if (action_navigation->m_event) {
        mem_free(action_navigation->m_module->m_alloc, action_navigation->m_event);
        action_navigation->m_event = NULL;
    }

    if (action_navigation->m_condition) {
        mem_free(action_navigation->m_module->m_alloc, action_navigation->m_condition);
        action_navigation->m_condition = NULL;
    }

    action_navigation->m_event = cpe_str_mem_dup(action_navigation->m_module->m_alloc, event);
    if (condition) {
        action_navigation->m_condition = cpe_str_mem_dup(action_navigation->m_module->m_alloc, condition);
    }
    
    return 0;
}

void ui_sprite_ui_action_navigation_set_navigation(ui_sprite_ui_action_navigation_t action_navigation, plugin_ui_navigation_t navigation) {
    action_navigation->m_navigation = navigation;
}

const char * ui_sprite_ui_action_navigation_event(ui_sprite_ui_action_navigation_t action_navigation) {
    return action_navigation->m_event;
}

const char * ui_sprite_ui_action_navigation_condition(ui_sprite_ui_action_navigation_t action_navigation) {
    return action_navigation->m_condition;
}

static void ui_sprite_ui_action_navigation_on_event(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_fsm_action_t fsm_action = ctx;
    ui_sprite_ui_action_navigation_t action_navigation = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_ui_module_t module = action_navigation->m_module;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_ui_env_t env = ui_sprite_ui_env_find(world);
    plugin_ui_phase_node_t phase_node;
    plugin_ui_state_node_t state_node;
    struct dr_data data;

    if (env == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_navigation_on_event: en sprite env!");
        return;
    }

    assert(action_navigation->m_navigation);

    if (action_navigation->m_condition) {
        uint8_t result;
        if (ui_sprite_fsm_action_check_calc_bool(&result, action_navigation->m_condition, fsm_action, NULL, module->m_em) != 0) {
            CPE_ERROR(module->m_em, "ui_sprite_ui_action_navigation_on_event: calc condition %s fail!", action_navigation->m_condition);
            return;
        }

        if (!result) return;
    }

    phase_node = plugin_ui_phase_node_current(env->m_env);
    if (phase_node == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_navigation_on_event: no current phase!");
        return;
    }

    state_node = plugin_ui_state_node_current(phase_node);
    if (state_node == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_navigation_on_event: no current state!");
        return;
    }
    
    data.m_meta = evt->meta;
    data.m_data = (void*)evt->data;
    data.m_size = evt->size;
    plugin_ui_navigation_execute(action_navigation->m_navigation, state_node, &data);
}

static int ui_sprite_ui_action_navigation_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_navigation_t action_navigation = ui_sprite_fsm_action_data(fsm_action);

    if (action_navigation->m_event == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_navigation_enter: no event configured!");
        return -1;
    }

    if (action_navigation->m_navigation == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_navigation_enter: no navigation configured!");
        return -1;
    }

    if (ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self,
            action_navigation->m_event, ui_sprite_ui_action_navigation_on_event, fsm_action)
        != 0)
    {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_navigation_enter: add event handler fail!");
        return -1;
    }
    
    return 0;
}

static void ui_sprite_ui_action_navigation_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    //ui_sprite_ui_action_navigation_t action_navigation = ui_sprite_fsm_action_data(fsm_action);
}

static int ui_sprite_ui_action_navigation_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_action_navigation_t action_navigation = ui_sprite_fsm_action_data(fsm_action);
    action_navigation->m_module = ctx;
    action_navigation->m_event = NULL;
    action_navigation->m_condition = NULL;
    action_navigation->m_navigation = NULL;
    return 0;
}

static void ui_sprite_ui_action_navigation_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t modue = ctx;
    ui_sprite_ui_action_navigation_t action_navigation = ui_sprite_fsm_action_data(fsm_action);
    
    if (action_navigation->m_event) {
        mem_free(modue->m_alloc, action_navigation->m_event);
        action_navigation->m_event = NULL;
    }

    if (action_navigation->m_condition) {
        mem_free(modue->m_alloc, action_navigation->m_condition);
        action_navigation->m_condition = NULL;
    }
}

static int ui_sprite_ui_action_navigation_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_ui_module_t modue = ctx;    
	ui_sprite_ui_action_navigation_t to_action_navigation = ui_sprite_fsm_action_data(to);
	ui_sprite_ui_action_navigation_t from_action_navigation = ui_sprite_fsm_action_data(from);

	if (ui_sprite_ui_action_navigation_init(to, ctx)) return -1;

    if (from_action_navigation->m_event) {
        to_action_navigation->m_event = cpe_str_mem_dup(modue->m_alloc, from_action_navigation->m_event);
    }

    if (from_action_navigation->m_condition) {
        to_action_navigation->m_condition = cpe_str_mem_dup(modue->m_alloc, from_action_navigation->m_condition);
    }

    to_action_navigation->m_navigation = from_action_navigation->m_navigation;
    
    return 0;
}

int ui_sprite_ui_action_navigation_regist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_UI_ACTION_NAVIGATION_NAME, sizeof(struct ui_sprite_ui_action_navigation));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ui enable emitter register: meta create fail",
            ui_sprite_ui_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ui_action_navigation_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ui_action_navigation_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ui_action_navigation_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ui_action_navigation_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ui_action_navigation_clear, module);

    return 0;
}

void ui_sprite_ui_action_navigation_unregist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_UI_ACTION_NAVIGATION_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_ui_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_UI_ACTION_NAVIGATION_NAME);
}

const char * UI_SPRITE_UI_ACTION_NAVIGATION_NAME = "ui-navigation";
