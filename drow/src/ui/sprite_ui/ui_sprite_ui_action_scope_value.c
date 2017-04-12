#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "plugin/ui/plugin_ui_popup.h"
#include "plugin/ui/plugin_ui_popup_def.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui_sprite_ui_action_scope_value_i.h"
#include "ui_sprite_ui_env_i.h"

ui_sprite_ui_action_scope_value_t ui_sprite_ui_action_scope_value_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_UI_ACTION_SCOPE_VALUE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ui_action_scope_value_free(ui_sprite_ui_action_scope_value_t action_scope_value) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(action_scope_value);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_ui_action_scope_value_set_control(ui_sprite_ui_action_scope_value_t action_scope_value, const char * control) {
    assert(control);

    if (action_scope_value->m_cfg_control) {
        mem_free(action_scope_value->m_module->m_alloc, action_scope_value->m_cfg_control);
        action_scope_value->m_cfg_control = NULL;
    }

    action_scope_value->m_cfg_control = cpe_str_mem_dup(action_scope_value->m_module->m_alloc, control);
    
    return 0;
}

const char * ui_sprite_ui_action_scope_value_name(ui_sprite_ui_action_scope_value_t action_scope_value) {
    return action_scope_value->m_cfg_control;
}

static int ui_sprite_ui_action_scope_value_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_scope_value_t action_scope_value = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (action_scope_value->m_cfg_control == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): show popup: no popup name configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    return 0;
}

static void ui_sprite_ui_action_scope_value_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    /* ui_sprite_ui_module_t module = ctx; */
    /* ui_sprite_ui_action_scope_value_t action_scope_value = ui_sprite_fsm_action_data(fsm_action); */
}

static int ui_sprite_ui_action_scope_value_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_action_scope_value_t action_scope_value = ui_sprite_fsm_action_data(fsm_action);
    action_scope_value->m_module = ctx;
    action_scope_value->m_cfg_control = NULL;
    TAILQ_INIT(&action_scope_value->m_items);
    return 0;
}

static void ui_sprite_ui_action_scope_value_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t modue = ctx;
    ui_sprite_ui_action_scope_value_t action_scope_value = ui_sprite_fsm_action_data(fsm_action);

    if (action_scope_value->m_cfg_control) {
        mem_free(modue->m_alloc, action_scope_value->m_cfg_control);
        action_scope_value->m_cfg_control = NULL;
    }

    while(!TAILQ_EMPTY(&action_scope_value->m_items)) {
        ui_sprite_ui_action_scope_value_item_free(action_scope_value, TAILQ_FIRST(&action_scope_value->m_items));
    }
}

static int ui_sprite_ui_action_scope_value_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_ui_module_t modue = ctx;    
	ui_sprite_ui_action_scope_value_t to_action_scope_value = ui_sprite_fsm_action_data(to);
	ui_sprite_ui_action_scope_value_t from_action_scope_value = ui_sprite_fsm_action_data(from);
    ui_sprite_ui_action_scope_value_item_t from_item;

	if (ui_sprite_ui_action_scope_value_init(to, ctx)) return -1;

    if (from_action_scope_value->m_cfg_control) {
        to_action_scope_value->m_cfg_control = cpe_str_mem_dup(modue->m_alloc, from_action_scope_value->m_cfg_control);
    }

    TAILQ_FOREACH(from_item, &from_action_scope_value->m_items, m_next) {
        ui_sprite_ui_action_scope_value_item_create(to_action_scope_value, from_item->m_cfg_name, from_item->m_cfg_value);
    }

    return 0;
}

ui_sprite_ui_action_scope_value_item_t
ui_sprite_ui_action_scope_value_item_create(
    ui_sprite_ui_action_scope_value_t scope_value, const char * cfg_name, const char * cfg_value)
{
    ui_sprite_ui_module_t module = scope_value->m_module;
    ui_sprite_ui_action_scope_value_item_t item;
    size_t name_len = strlen(cfg_name) + 1;
    size_t value_len = strlen(cfg_value) + 1;
                             
    item = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_ui_action_scope_value_item) + name_len + value_len);
    if (item == NULL) {
        CPE_ERROR(module->m_em, "%s: create action_scope_value action: alloc item fail!", ui_sprite_ui_module_name(module));
        return NULL;
    }

    item->m_cfg_name = (void*)(item + 1);
    memcpy(item->m_cfg_name, cfg_name, name_len);

    item->m_cfg_value = (void*)(item->m_cfg_name + name_len);
    memcpy(item->m_cfg_value, cfg_value, value_len);

    TAILQ_INSERT_TAIL(&scope_value->m_items, item, m_next);

    return item;
}

void ui_sprite_ui_action_scope_value_item_free(ui_sprite_ui_action_scope_value_t scope_value, ui_sprite_ui_action_scope_value_item_t item) {
    ui_sprite_ui_module_t module = scope_value->m_module;
    TAILQ_REMOVE(&scope_value->m_items, item, m_next);
    mem_free(module->m_alloc, item);
}

static ui_sprite_fsm_action_t
ui_sprite_ui_action_scope_value_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_scope_value_t action_scope_value = ui_sprite_ui_action_scope_value_create(fsm_state, name);
    const char * str_value;
    
    if (action_scope_value == NULL) {
        CPE_ERROR(module->m_em, "%s: create action_scope_value action: create fail!", ui_sprite_ui_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "control", NULL))) {
        if (ui_sprite_ui_action_scope_value_set_control(action_scope_value, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create action_scope_value action: set control name %s fail!",
                ui_sprite_ui_module_name(module), str_value);
            ui_sprite_ui_action_scope_value_free(action_scope_value);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create action_scope_value action: control not configured!", ui_sprite_ui_module_name(module));
        ui_sprite_ui_action_scope_value_free(action_scope_value);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(action_scope_value);
}

int ui_sprite_ui_action_scope_value_regist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_UI_ACTION_SCOPE_VALUE_NAME, sizeof(struct ui_sprite_ui_action_scope_value));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ui enable emitter register: meta create fail",
            ui_sprite_ui_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ui_action_scope_value_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ui_action_scope_value_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ui_action_scope_value_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ui_action_scope_value_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ui_action_scope_value_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_UI_ACTION_SCOPE_VALUE_NAME, ui_sprite_ui_action_scope_value_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_ui_action_scope_value_unregist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_UI_ACTION_SCOPE_VALUE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_ui_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_UI_ACTION_SCOPE_VALUE_NAME);
}

const char * UI_SPRITE_UI_ACTION_SCOPE_VALUE_NAME = "ui-scope-value";
