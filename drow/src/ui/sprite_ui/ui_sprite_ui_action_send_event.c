#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "plugin/ui/plugin_ui_page.h"
#include "plugin/ui/plugin_ui_phase_node.h"
#include "plugin/ui/plugin_ui_state_node.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_ui_action_send_event_i.h"
#include "ui_sprite_ui_env_i.h"

ui_sprite_ui_action_send_event_t ui_sprite_ui_action_send_event_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_UI_ACTION_SEND_EVENT_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ui_action_send_event_free(ui_sprite_ui_action_send_event_t action_send_event) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(action_send_event);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_ui_action_send_event_set_page_name(ui_sprite_ui_action_send_event_t action_send_event, const char * page_name) {
    assert(page_name);

    if (action_send_event->m_page_name) {
        mem_free(action_send_event->m_module->m_alloc, action_send_event->m_page_name);
        action_send_event->m_page_name = NULL;
    }

    action_send_event->m_page_name = cpe_str_mem_dup(action_send_event->m_module->m_alloc, page_name);
    
    return 0;
}

int ui_sprite_ui_action_send_event_set_on_enter(ui_sprite_ui_action_send_event_t action_send_event, const char * on_enter) {
    assert(on_enter);

    if (action_send_event->m_on_enter) {
        mem_free(action_send_event->m_module->m_alloc, action_send_event->m_on_enter);
        action_send_event->m_on_enter = NULL;
    }

    if (on_enter) {
        action_send_event->m_on_enter = cpe_str_mem_dup_trim(action_send_event->m_module->m_alloc, cpe_str_trim_head((char*)on_enter));
        if (action_send_event->m_on_enter == NULL) return -1;
    }
    else {
        on_enter = NULL;
    }
    
    return 0;
}

int ui_sprite_ui_action_send_event_set_on_exit(ui_sprite_ui_action_send_event_t action_send_event, const char * on_exit) {
    assert(on_exit);

    if (action_send_event->m_on_exit) {
        mem_free(action_send_event->m_module->m_alloc, action_send_event->m_on_exit);
        action_send_event->m_on_exit = NULL;
    }

    if (on_exit) {
        action_send_event->m_on_exit = cpe_str_mem_dup(action_send_event->m_module->m_alloc, cpe_str_trim_head((char*)on_exit));
        if (action_send_event->m_on_exit == NULL) return -1;
        
        * cpe_str_trim_tail(
            action_send_event->m_on_exit + strlen(action_send_event->m_on_exit),
            action_send_event->m_on_exit) = 0;
    }
    else {
        on_exit = NULL;
    }
    
    return 0;
}

static int ui_sprite_ui_action_send_event_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_send_event_t action_send_event = ui_sprite_fsm_action_data(fsm_action);

    if (action_send_event->m_on_enter) {
        struct ui_sprite_fsm_addition_source_ctx ctx;
        dr_data_source_t data_source = NULL;
        ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
        
        if (action_send_event->m_page_name == NULL) {
            plugin_ui_env_build_and_send_event(module->m_env->m_env, action_send_event->m_on_enter, data_source);
        }
        else {
            plugin_ui_page_t page = plugin_ui_page_find(module->m_env->m_env, action_send_event->m_page_name);
            if (page == NULL) {
                ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
                CPE_ERROR(
                    module->m_em, "entity %d(%s): %s: page not exist!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), action_send_event->m_page_name);
                return -1;
            }
            
            plugin_ui_page_build_and_send_event(page, action_send_event->m_on_enter, data_source);
        }
        
    }

    return 0;
}

static void ui_sprite_ui_action_send_event_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_send_event_t action_send_event = ui_sprite_fsm_action_data(fsm_action);

    if (action_send_event->m_on_exit) {
        struct ui_sprite_fsm_addition_source_ctx ctx;
        dr_data_source_t data_source = NULL;
        ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
        
        if (action_send_event->m_page_name == NULL) {
            plugin_ui_env_build_and_send_event(module->m_env->m_env, action_send_event->m_on_exit, data_source);
        }
        else {
            plugin_ui_page_t page = plugin_ui_page_find(module->m_env->m_env, action_send_event->m_page_name);
            if (page == NULL) {
                ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
                CPE_ERROR(
                    module->m_em, "entity %d(%s): %s: page not exist!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), action_send_event->m_page_name);
                return;
            }
            
            plugin_ui_page_build_and_send_event(page, action_send_event->m_on_exit, data_source);
        }
    }
}

static int ui_sprite_ui_action_send_event_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_action_send_event_t action_send_event = ui_sprite_fsm_action_data(fsm_action);
    action_send_event->m_module = ctx;
    action_send_event->m_page_name = NULL;
    action_send_event->m_on_enter = NULL;
    action_send_event->m_on_exit = NULL;    
    return 0;
}

static void ui_sprite_ui_action_send_event_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t modue = ctx;
    ui_sprite_ui_action_send_event_t action_send_event = ui_sprite_fsm_action_data(fsm_action);

    if (action_send_event->m_page_name) {
        mem_free(modue->m_alloc, action_send_event->m_page_name);
        action_send_event->m_page_name = NULL;
    }

    if (action_send_event->m_on_enter) {
        mem_free(modue->m_alloc, action_send_event->m_on_enter);
        action_send_event->m_on_enter = NULL;
    }

    if (action_send_event->m_on_exit) {
        mem_free(modue->m_alloc, action_send_event->m_on_exit);
        action_send_event->m_on_exit = NULL;
    }
}

static int ui_sprite_ui_action_send_event_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_ui_module_t modue = ctx;    
	ui_sprite_ui_action_send_event_t to_action_send_event = ui_sprite_fsm_action_data(to);
	ui_sprite_ui_action_send_event_t from_action_send_event = ui_sprite_fsm_action_data(from);

	if (ui_sprite_ui_action_send_event_init(to, ctx)) return -1;

    if (from_action_send_event->m_page_name) {
        to_action_send_event->m_page_name = cpe_str_mem_dup(modue->m_alloc, from_action_send_event->m_page_name);
    }

    if (from_action_send_event->m_on_enter) {
        to_action_send_event->m_on_enter = cpe_str_mem_dup(modue->m_alloc, from_action_send_event->m_on_enter);
    }

    if (from_action_send_event->m_on_exit) {
        to_action_send_event->m_on_exit = cpe_str_mem_dup(modue->m_alloc, from_action_send_event->m_on_exit);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_ui_action_send_event_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_send_event_t action_send_event = ui_sprite_ui_action_send_event_create(fsm_state, name);
    const char * str_value;
    
    if (action_send_event == NULL) {
        CPE_ERROR(module->m_em, "%s: create action_send_event action: create fail!", ui_sprite_ui_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "page", NULL))) {
        if (ui_sprite_ui_action_send_event_set_page_name(action_send_event, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create action_send_event action: set page name %s fail!",
                ui_sprite_ui_module_name(module), str_value);
            ui_sprite_ui_action_send_event_free(action_send_event);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "on-enter", NULL))) {
        if (ui_sprite_ui_action_send_event_set_on_enter(action_send_event, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create action_send_event action: set on-enter %s fail!",
                ui_sprite_ui_module_name(module), str_value);
            ui_sprite_ui_action_send_event_free(action_send_event);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "on-exit", NULL))) {
        if (ui_sprite_ui_action_send_event_set_on_exit(action_send_event, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create action_send_event action: set on-exit %s fail!",
                ui_sprite_ui_module_name(module), str_value);
            ui_sprite_ui_action_send_event_free(action_send_event);
            return NULL;
        }
    }

    return ui_sprite_fsm_action_from_data(action_send_event);
}

int ui_sprite_ui_action_send_event_regist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_UI_ACTION_SEND_EVENT_NAME, sizeof(struct ui_sprite_ui_action_send_event));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ui enable emitter register: meta create fail",
            ui_sprite_ui_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ui_action_send_event_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ui_action_send_event_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ui_action_send_event_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ui_action_send_event_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ui_action_send_event_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_UI_ACTION_SEND_EVENT_NAME, ui_sprite_ui_action_send_event_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_ui_action_send_event_unregist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_UI_ACTION_SEND_EVENT_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_ui_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_UI_ACTION_SEND_EVENT_NAME);
}

const char * UI_SPRITE_UI_ACTION_SEND_EVENT_NAME = "ui-send-event";
