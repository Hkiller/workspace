#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_basic_set_attrs_i.h"

ui_sprite_basic_set_attrs_t ui_sprite_basic_set_attrs_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_BASIC_SET_ATTRS_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_basic_set_attrs_free(ui_sprite_basic_set_attrs_t set_attrs) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(set_attrs);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_basic_set_attrs_set_setter(ui_sprite_basic_set_attrs_t set_attrs, const char * setter) {
    ui_sprite_basic_module_t module = set_attrs->m_module;

    if (set_attrs->m_setter) mem_free(module->m_alloc, set_attrs->m_setter);

    if (setter) {
        set_attrs->m_setter = cpe_str_mem_dup(module->m_alloc, setter);
        return set_attrs->m_setter == NULL ? -1 : 0;
    }
    else {
        set_attrs->m_setter = NULL;
        return 0;
    }
}

const char * ui_sprite_basic_set_attrs_setter(ui_sprite_basic_set_attrs_t set_attrs) {
    return set_attrs->m_setter;
}

static int ui_sprite_basic_set_attrs_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_set_attrs_t set_attrs = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (set_attrs->m_setter == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): set attrs: attrs name not set!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (ui_sprite_fsm_action_bulk_set_attrs(fsm_action, set_attrs->m_setter, NULL) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): set attrs: set attrs fail, %s!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            set_attrs->m_setter);
        return -1;
    }

    return 0;
}

static void ui_sprite_basic_set_attrs_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_basic_set_attrs_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_set_attrs_t set_attrs = ui_sprite_fsm_action_data(fsm_action);
    set_attrs->m_module = ctx;
	set_attrs->m_setter = NULL;
    return 0;
}

static void ui_sprite_basic_set_attrs_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_set_attrs_t set_attrs = ui_sprite_fsm_action_data(fsm_action);

    if (set_attrs->m_setter) {
        mem_free(module->m_alloc, set_attrs->m_setter);
        set_attrs->m_setter = NULL;
    }
}

static int ui_sprite_basic_set_attrs_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_set_attrs_t to_set_attrs = ui_sprite_fsm_action_data(to);
    ui_sprite_basic_set_attrs_t from_set_attrs = ui_sprite_fsm_action_data(from);

    if (ui_sprite_basic_set_attrs_init(to, ctx)) return -1;

    if (from_set_attrs->m_setter) {
        to_set_attrs->m_setter = cpe_str_mem_dup(module->m_alloc, from_set_attrs->m_setter);
    }

    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_basic_set_attrs_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_set_attrs_t set_attrs;
    const char * setter;

    setter = cfg_get_string(cfg, "setter", NULL);
    if (setter == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create set_attrs action: setter not configured",
            ui_sprite_basic_module_name(module));
        return NULL;
    }

    set_attrs = ui_sprite_basic_set_attrs_create(fsm_state, name);
    if (set_attrs == NULL) {
        CPE_ERROR(module->m_em, "%s: create set_attrs action: create fail!", ui_sprite_basic_module_name(module));
        return NULL;
    }

    if (ui_sprite_basic_set_attrs_set_setter(set_attrs, setter) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create set_attrs action: set setter %s fail",
            ui_sprite_basic_module_name(module), setter);
        ui_sprite_basic_set_attrs_free(set_attrs);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(set_attrs);
}

int ui_sprite_basic_set_attrs_regist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_BASIC_SET_ATTRS_NAME, sizeof(struct ui_sprite_basic_set_attrs));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_basic_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_basic_set_attrs_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_basic_set_attrs_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_basic_set_attrs_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_basic_set_attrs_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_basic_set_attrs_clear, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_BASIC_SET_ATTRS_NAME, ui_sprite_basic_set_attrs_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_basic_set_attrs_unregist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_BASIC_SET_ATTRS_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_basic_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_BASIC_SET_ATTRS_NAME);
    }
}

const char * UI_SPRITE_BASIC_SET_ATTRS_NAME = "set-attrs";

