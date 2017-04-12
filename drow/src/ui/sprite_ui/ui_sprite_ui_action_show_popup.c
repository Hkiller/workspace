#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "plugin/ui/plugin_ui_popup.h"
#include "plugin/ui/plugin_ui_popup_def.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui_sprite_ui_action_show_popup_i.h"
#include "ui_sprite_ui_env_i.h"

ui_sprite_ui_action_show_popup_t ui_sprite_ui_action_show_popup_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_UI_ACTION_SHOW_POPUP_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ui_action_show_popup_free(ui_sprite_ui_action_show_popup_t action_show_popup) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(action_show_popup);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_ui_action_show_popup_set_popup_name(ui_sprite_ui_action_show_popup_t action_show_popup, const char * popup_name) {
    assert(popup_name);

    if (action_show_popup->m_cfg_popup_name) {
        mem_free(action_show_popup->m_module->m_alloc, action_show_popup->m_cfg_popup_name);
        action_show_popup->m_cfg_popup_name = NULL;
    }

    action_show_popup->m_cfg_popup_name = cpe_str_mem_dup(action_show_popup->m_module->m_alloc, popup_name);
    
    return 0;
}

const char * ui_sprite_ui_action_show_popup_name(ui_sprite_ui_action_show_popup_t action_show_popup) {
    return action_show_popup->m_cfg_popup_name;
}

static int ui_sprite_ui_action_show_popup_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_show_popup_t action_show_popup = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    plugin_ui_popup_def_t popup_def;
    plugin_ui_popup_t popup;

    assert(action_show_popup->m_popup == NULL);

    if (action_show_popup->m_cfg_popup_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): show popup: no popup name configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    popup_def = plugin_ui_popup_def_find(module->m_env->m_env, action_show_popup->m_cfg_popup_name);
    if (popup_def == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): show popup: popup %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), action_show_popup->m_cfg_popup_name);
        return -1;
    }

    popup = plugin_ui_popup_def_create_popup(popup_def);
    if (popup == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): show popup: popup %s create fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), action_show_popup->m_cfg_popup_name);
        return -1;
    }

    plugin_ui_popup_set_visible(popup, 1);

    action_show_popup->m_popup = popup;
    
    if (ui_sprite_fsm_action_life_circle(fsm_action) == ui_sprite_fsm_action_life_circle_working) {
        ui_sprite_fsm_action_start_update(fsm_action);
    }

    return 0;
}

static void ui_sprite_ui_action_show_popup_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_show_popup_t action_show_popup = ui_sprite_fsm_action_data(fsm_action);
    struct plugin_ui_popup_it popup_it;
    plugin_ui_popup_t popup;

    assert(action_show_popup->m_popup);

    plugin_ui_env_popups(module->m_env->m_env, &popup_it);

    while((popup = plugin_ui_popup_it_next(&popup_it))) {
        if (popup == action_show_popup->m_popup) return;
    }

    action_show_popup->m_popup = NULL;
    ui_sprite_fsm_action_stop_update(fsm_action);
}

static void ui_sprite_ui_action_show_popup_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_show_popup_t action_show_popup = ui_sprite_fsm_action_data(fsm_action);

    if(action_show_popup->m_popup) {
        struct plugin_ui_popup_it popup_it;
        plugin_ui_popup_t popup;

        plugin_ui_env_popups(module->m_env->m_env, &popup_it);
        while((popup = plugin_ui_popup_it_next(&popup_it))) {
            if (popup == action_show_popup->m_popup) {
                plugin_ui_popup_set_visible(popup, 0);
                break;
            }
        }

        action_show_popup->m_popup = NULL;
    }
}

static int ui_sprite_ui_action_show_popup_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_action_show_popup_t action_show_popup = ui_sprite_fsm_action_data(fsm_action);
    action_show_popup->m_module = ctx;
    action_show_popup->m_cfg_popup_name = NULL;
    action_show_popup->m_popup = NULL;
    return 0;
}

static void ui_sprite_ui_action_show_popup_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t modue = ctx;
    ui_sprite_ui_action_show_popup_t action_show_popup = ui_sprite_fsm_action_data(fsm_action);

    assert(action_show_popup->m_popup == NULL);
    
    if (action_show_popup->m_cfg_popup_name) {
        mem_free(modue->m_alloc, action_show_popup->m_cfg_popup_name);
        action_show_popup->m_cfg_popup_name = NULL;
    }
}

static int ui_sprite_ui_action_show_popup_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_ui_module_t modue = ctx;    
	ui_sprite_ui_action_show_popup_t to_action_show_popup = ui_sprite_fsm_action_data(to);
	ui_sprite_ui_action_show_popup_t from_action_show_popup = ui_sprite_fsm_action_data(from);

	if (ui_sprite_ui_action_show_popup_init(to, ctx)) return -1;

    if (from_action_show_popup->m_cfg_popup_name) {
        to_action_show_popup->m_cfg_popup_name = cpe_str_mem_dup(modue->m_alloc, from_action_show_popup->m_cfg_popup_name);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_ui_action_show_popup_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_show_popup_t action_show_popup = ui_sprite_ui_action_show_popup_create(fsm_state, name);
    const char * popup_name;
    
    if (action_show_popup == NULL) {
        CPE_ERROR(module->m_em, "%s: create action_show_popup action: create fail!", ui_sprite_ui_module_name(module));
        return NULL;
    }

    popup_name = cfg_get_string(cfg, "popup", NULL);
    if (popup_name == NULL) {
        CPE_ERROR(module->m_em, "%s: create action_show_popup action: popup not configured!", ui_sprite_ui_module_name(module));
        ui_sprite_ui_action_show_popup_free(action_show_popup);
        return NULL;
    }

    if (ui_sprite_ui_action_show_popup_set_popup_name(action_show_popup, popup_name) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create action_show_popup action: set popup name %s fail!",
            ui_sprite_ui_module_name(module), popup_name);
        ui_sprite_ui_action_show_popup_free(action_show_popup);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(action_show_popup);
}

int ui_sprite_ui_action_show_popup_regist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_UI_ACTION_SHOW_POPUP_NAME, sizeof(struct ui_sprite_ui_action_show_popup));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ui enable emitter register: meta create fail",
            ui_sprite_ui_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ui_action_show_popup_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ui_action_show_popup_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ui_action_show_popup_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ui_action_show_popup_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ui_action_show_popup_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_ui_action_show_popup_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_UI_ACTION_SHOW_POPUP_NAME, ui_sprite_ui_action_show_popup_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_ui_action_show_popup_unregist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_UI_ACTION_SHOW_POPUP_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_ui_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_UI_ACTION_SHOW_POPUP_NAME);
}

const char * UI_SPRITE_UI_ACTION_SHOW_POPUP_NAME = "ui-show-popup";
