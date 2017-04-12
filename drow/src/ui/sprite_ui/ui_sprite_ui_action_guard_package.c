#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "plugin/package/plugin_package_package.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_ui_action_guard_package_i.h"
#include "ui_sprite_ui_env_i.h"

ui_sprite_ui_action_guard_package_t
ui_sprite_ui_action_guard_package_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_UI_ACTION_GUARD_PACKAGE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ui_action_guard_package_free(ui_sprite_ui_action_guard_package_t action_guard_package) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(action_guard_package);
    ui_sprite_fsm_action_free(fsm_action);
}

void ui_sprite_ui_action_guard_package_set_package(ui_sprite_ui_action_guard_package_t guard_package, const char * package) {
    if (guard_package->m_cfg_package) {
        mem_free(guard_package->m_module->m_alloc, guard_package->m_cfg_package);
    }

    if (package) {
        guard_package->m_cfg_package = cpe_str_mem_dup_trim(guard_package->m_module->m_alloc, package);
    }
    else {
        guard_package->m_cfg_package = NULL;
    }
}

static int ui_sprite_ui_action_guard_package_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_guard_package_t guard_package = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    const char * package_name;
    
    if (guard_package->m_cfg_package == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): guard-package: package not configured",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    package_name = ui_sprite_fsm_action_check_calc_str(gd_app_tmp_buffer(module->m_app), guard_package->m_cfg_package, fsm_action, NULL, module->m_em);
    if (package_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): guard-package: calc package from %s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), guard_package->m_cfg_package);
        return -1;
    }
    
    guard_package->m_package = plugin_package_package_find(plugin_ui_env_package_mgr(module->m_env->m_env), package_name);
    if (guard_package->m_package == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): guard-package: package %s not exist",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), package_name);
        return -1;
    }

    if (plugin_package_package_load_sync(guard_package->m_package) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): guard-package: package %s load fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), package_name);
        guard_package->m_package = NULL;
        return -1;
    }
    
    return 0;
}

static void ui_sprite_ui_action_guard_package_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_action_guard_package_t guard_package = ui_sprite_fsm_action_data(fsm_action);
    if (guard_package->m_package) {
        plugin_package_package_unload(guard_package->m_package);
        guard_package->m_package = NULL;
    }
}

static int ui_sprite_ui_action_guard_package_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_action_guard_package_t guard_package = ui_sprite_fsm_action_data(fsm_action);
    guard_package->m_module = ctx;
    guard_package->m_cfg_package = NULL;
    guard_package->m_package = NULL;
    return 0;
}

static void ui_sprite_ui_action_guard_package_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_guard_package_t guard_package = ui_sprite_fsm_action_data(fsm_action);

    if (guard_package->m_cfg_package) {
        mem_free(module->m_alloc, guard_package->m_cfg_package);
        guard_package->m_cfg_package = NULL;
    }
}

static int ui_sprite_ui_action_guard_package_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_guard_package_t to_guard_package = ui_sprite_fsm_action_data(to);
    ui_sprite_ui_action_guard_package_t from_guard_package = ui_sprite_fsm_action_data(from);
    
	if (ui_sprite_ui_action_guard_package_init(to, ctx)) return -1;

    if (from_guard_package->m_cfg_package) {
        to_guard_package->m_cfg_package = cpe_str_mem_dup(module->m_alloc, from_guard_package->m_cfg_package);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_ui_action_guard_package_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_guard_package_t guard_package = ui_sprite_ui_action_guard_package_create(fsm_state, name);
    const char * str_value;

    if (guard_package == NULL) {
        CPE_ERROR(module->m_em, "%s: create guard-package: create fail!", ui_sprite_ui_module_name(module));
        return NULL;
    }

    str_value = cfg_get_string(cfg, "package", NULL);
    if (str_value == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create guard-package: package not configured!",
            ui_sprite_ui_module_name(module));
        ui_sprite_ui_action_guard_package_free(guard_package);
        return NULL;
    }
    ui_sprite_ui_action_guard_package_set_package(guard_package, str_value);

    return ui_sprite_fsm_action_from_data(guard_package);
}

int ui_sprite_ui_action_guard_package_regist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_UI_ACTION_GUARD_PACKAGE_NAME, sizeof(struct ui_sprite_ui_action_guard_package));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ui enable emitter register: meta create fail",
            ui_sprite_ui_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ui_action_guard_package_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ui_action_guard_package_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ui_action_guard_package_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ui_action_guard_package_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ui_action_guard_package_clear, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(
                module->m_loader,
                UI_SPRITE_UI_ACTION_GUARD_PACKAGE_NAME,
                ui_sprite_ui_action_guard_package_load, module) != 0)
        {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_ui_action_guard_package_unregist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_UI_ACTION_GUARD_PACKAGE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_ui_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_UI_ACTION_GUARD_PACKAGE_NAME);
}

const char * UI_SPRITE_UI_ACTION_GUARD_PACKAGE_NAME = "ui-guard-package";

