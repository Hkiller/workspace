#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_basic_guard_attrs_i.h"

ui_sprite_basic_guard_attrs_t ui_sprite_basic_guard_attrs_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_BASIC_GUARD_ATTRS_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_basic_guard_attrs_free(ui_sprite_basic_guard_attrs_t guard_attrs) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(guard_attrs);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_basic_guard_attrs_guard_set_on_enter(ui_sprite_basic_guard_attrs_t guard_attrs, const char * on_enter) {
    ui_sprite_basic_module_t module = guard_attrs->m_module;

    if (guard_attrs->m_on_enter) mem_free(module->m_alloc, guard_attrs->m_on_enter);

    if (on_enter) {
        guard_attrs->m_on_enter = cpe_str_mem_dup(module->m_alloc, on_enter);
        return guard_attrs->m_on_enter == NULL ? -1 : 0;
    }
    else {
        guard_attrs->m_on_enter = NULL;
        return 0;
    }
}

const char * ui_sprite_basic_guard_attrs_on_enter(ui_sprite_basic_guard_attrs_t guard_attrs) {
    return guard_attrs->m_on_enter;
}

int ui_sprite_basic_guard_attrs_guard_set_on_exit(ui_sprite_basic_guard_attrs_t guard_attrs, const char * on_exit) {
    ui_sprite_basic_module_t module = guard_attrs->m_module;

    if (guard_attrs->m_on_exit) mem_free(module->m_alloc, guard_attrs->m_on_exit);

    if (on_exit) {
        guard_attrs->m_on_exit = cpe_str_mem_dup(module->m_alloc, on_exit);
        return guard_attrs->m_on_exit == NULL ? -1 : 0;
    }
    else {
        guard_attrs->m_on_exit = NULL;
        return 0;
    }
}

const char * ui_sprite_basic_guard_attrs_on_exit(ui_sprite_basic_guard_attrs_t guard_attrs) {
    return guard_attrs->m_on_exit;
}

static int ui_sprite_basic_guard_attrs_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_guard_attrs_t guard_attrs = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (guard_attrs->m_on_enter == NULL) {
        if (ui_sprite_fsm_action_bulk_set_attrs(fsm_action, guard_attrs->m_on_enter, NULL) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): guard attrs: guard attrs fail, %s!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                guard_attrs->m_on_enter);
            return -1;
        }
    }
    
    return 0;
}

static void ui_sprite_basic_guard_attrs_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_guard_attrs_t guard_attrs = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (guard_attrs->m_on_exit == NULL) {
        if (ui_sprite_fsm_action_bulk_set_attrs(fsm_action, guard_attrs->m_on_exit, NULL) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): guard attrs: set on exit fail, %s!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                guard_attrs->m_on_exit);
        }
    }
}

static int ui_sprite_basic_guard_attrs_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_guard_attrs_t guard_attrs = ui_sprite_fsm_action_data(fsm_action);
    guard_attrs->m_module = ctx;
	guard_attrs->m_on_enter = NULL;
	guard_attrs->m_on_exit = NULL;
    return 0;
}

static void ui_sprite_basic_guard_attrs_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_guard_attrs_t guard_attrs = ui_sprite_fsm_action_data(fsm_action);

    if (guard_attrs->m_on_enter) {
        mem_free(module->m_alloc, guard_attrs->m_on_enter);
        guard_attrs->m_on_enter = NULL;
    }

    if (guard_attrs->m_on_exit) {
        mem_free(module->m_alloc, guard_attrs->m_on_exit);
        guard_attrs->m_on_exit = NULL;
    }
}

static int ui_sprite_basic_guard_attrs_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_guard_attrs_t to_guard_attrs = ui_sprite_fsm_action_data(to);
    ui_sprite_basic_guard_attrs_t from_guard_attrs = ui_sprite_fsm_action_data(from);

    if (ui_sprite_basic_guard_attrs_init(to, ctx)) return -1;

    if (from_guard_attrs->m_on_enter) {
        to_guard_attrs->m_on_enter = cpe_str_mem_dup(module->m_alloc, from_guard_attrs->m_on_enter);
    }

    if (from_guard_attrs->m_on_exit) {
        to_guard_attrs->m_on_exit = cpe_str_mem_dup(module->m_alloc, from_guard_attrs->m_on_exit);
    }

    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_basic_guard_attrs_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_guard_attrs_t guard_attrs;
    const char * str_value;

    guard_attrs = ui_sprite_basic_guard_attrs_create(fsm_state, name);
    if (guard_attrs == NULL) {
        CPE_ERROR(module->m_em, "%s: create guard_attrs action: create fail!", ui_sprite_basic_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "on-enter", NULL))) {
        if (ui_sprite_basic_guard_attrs_guard_set_on_enter(guard_attrs, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create guard_attrs action: set on enter %s fail",
                ui_sprite_basic_module_name(module), str_value);
            ui_sprite_basic_guard_attrs_free(guard_attrs);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "on-exit", NULL))) {
        if (ui_sprite_basic_guard_attrs_guard_set_on_exit(guard_attrs, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create guard_attrs action: set on exit %s fail",
                ui_sprite_basic_module_name(module), str_value);
            ui_sprite_basic_guard_attrs_free(guard_attrs);
            return NULL;
        }
    }
    
    return ui_sprite_fsm_action_from_data(guard_attrs);
}

int ui_sprite_basic_guard_attrs_regist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_BASIC_GUARD_ATTRS_NAME, sizeof(struct ui_sprite_basic_guard_attrs));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_basic_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_basic_guard_attrs_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_basic_guard_attrs_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_basic_guard_attrs_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_basic_guard_attrs_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_basic_guard_attrs_clear, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_BASIC_GUARD_ATTRS_NAME, ui_sprite_basic_guard_attrs_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_basic_guard_attrs_unregist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_BASIC_GUARD_ATTRS_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_basic_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_BASIC_GUARD_ATTRS_NAME);
    }
}

const char * UI_SPRITE_BASIC_GUARD_ATTRS_NAME = "guard-attrs";

