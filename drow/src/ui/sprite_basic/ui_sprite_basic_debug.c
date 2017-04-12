#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_basic_debug_i.h"

ui_sprite_basic_debug_t ui_sprite_basic_debug_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_BASIC_DEBUG_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_basic_debug_free(ui_sprite_basic_debug_t debug) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(debug);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_basic_debug_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_debug_t debug = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    debug->m_old_debug = ui_sprite_entity_debug(entity);
    ui_sprite_entity_set_debug(entity, 1);

    return 0;
}

static void ui_sprite_basic_debug_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_debug_t debug = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    ui_sprite_entity_set_debug(entity, debug->m_old_debug);
}

static ui_sprite_fsm_action_t ui_sprite_basic_debug_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_debug_t debug = ui_sprite_basic_debug_create(fsm_state, name);

    if (debug == NULL) {
        CPE_ERROR(module->m_em, "%s: create debug action: create fail!", ui_sprite_basic_module_name(module));
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(debug);
}

int ui_sprite_basic_debug_regist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_BASIC_DEBUG_NAME, sizeof(struct ui_sprite_basic_debug));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_basic_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_basic_debug_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_basic_debug_exit, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_BASIC_DEBUG_NAME, ui_sprite_basic_debug_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_basic_debug_unregist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_BASIC_DEBUG_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_basic_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_BASIC_DEBUG_NAME);
    }
}

const char * UI_SPRITE_BASIC_DEBUG_NAME = "debug";

