#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_chipmunk_with_runing_mode_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_with_runing_mode_t ui_sprite_chipmunk_with_runing_mode_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CHIPMUNK_WITH_RUNING_MODE_NAME);
    return fsm_action ? (ui_sprite_chipmunk_with_runing_mode_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_chipmunk_with_runing_mode_free(ui_sprite_chipmunk_with_runing_mode_t with_runing_mode) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(with_runing_mode);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_chipmunk_with_runing_mode_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_runing_mode_t with_runing_mode = (ui_sprite_chipmunk_with_runing_mode_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t chipmunk_obj;
    ui_sprite_chipmunk_obj_body_t body;
    char * str_runing_mode;
    uint8_t runing_mode;
    
    str_runing_mode = (char *)ui_sprite_fsm_action_check_calc_str(
        gd_app_tmp_buffer(module->m_app), with_runing_mode->m_cfg_runing_mode, fsm_action, NULL, module->m_em);
    if (str_runing_mode == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk-with-runing-mode: calc runing mode from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_runing_mode->m_cfg_runing_mode);
        goto ENTER_FAIL;
    }
    
    if (strcmp(str_runing_mode, "passive") == 0) {
        runing_mode = ui_sprite_chipmunk_runing_mode_passive;
    }
    else if (strcmp(str_runing_mode, "active") == 0) {
        runing_mode = ui_sprite_chipmunk_runing_mode_active;
    }
    else {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk-with-runing-mode: runing mode %s unknown!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_runing_mode);
        goto ENTER_FAIL;
    }

    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk-with-runing-mode: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    assert(with_runing_mode->m_body_name == NULL);
    if (with_runing_mode->m_cfg_body_name) {
        with_runing_mode->m_body_name =
            ui_sprite_fsm_action_check_calc_str_dup(
                module->m_alloc, with_runing_mode->m_cfg_body_name, fsm_action, NULL, module->m_em);
        if (with_runing_mode->m_body_name == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk-with-runing-mode: calc body from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_runing_mode->m_cfg_body_name);
            goto ENTER_FAIL;
        }
    }

    if (with_runing_mode->m_body_name) {
        body = ui_sprite_chipmunk_obj_body_find(chipmunk_obj, with_runing_mode->m_body_name);
        if (body == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk-with-runing-mode: body %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_runing_mode->m_body_name);
            goto ENTER_FAIL;
        }
    }
    else {
        body = chipmunk_obj->m_main_body;
        if (body == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with runing_mode: main body not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            goto ENTER_FAIL;
        }
    }

    with_runing_mode->m_saved_runing_mode = (ui_sprite_chipmunk_runing_mode_t)ui_sprite_chipmunk_obj_body_runing_mode(body);
    if (ui_sprite_chipmunk_obj_body_set_runing_mode(body, runing_mode) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with runing_mode: body set runing mode fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }
    
    return 0;

ENTER_FAIL:
    if (with_runing_mode->m_body_name) {
        mem_free(module->m_alloc, with_runing_mode->m_body_name);
        with_runing_mode->m_body_name = NULL;
    }

    return -1;
}

static void ui_sprite_chipmunk_with_runing_mode_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_runing_mode_t with_runing_mode = (ui_sprite_chipmunk_with_runing_mode_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t chipmunk_obj;
    ui_sprite_chipmunk_obj_body_t body = NULL;
    
    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with runing_mode: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (with_runing_mode->m_body_name) {
        body = ui_sprite_chipmunk_obj_body_find(chipmunk_obj, with_runing_mode->m_body_name);
        if (body == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with runing_mode: body %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_runing_mode->m_body_name);
        }
    }
    else {
        body = chipmunk_obj->m_main_body;
        if (body == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with runing_mode: main body not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        }
    }
    
    if (body) ui_sprite_chipmunk_obj_body_set_runing_mode(body, (uint8_t)&with_runing_mode->m_saved_runing_mode);

    if (with_runing_mode->m_body_name) {
        mem_free(module->m_alloc, with_runing_mode->m_body_name);
    }
}

static int ui_sprite_chipmunk_with_runing_mode_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_with_runing_mode_t with_runing_mode = (ui_sprite_chipmunk_with_runing_mode_t)ui_sprite_fsm_action_data(fsm_action);
    with_runing_mode->m_module = (ui_sprite_chipmunk_module_t)ctx;
    with_runing_mode->m_cfg_body_name = NULL;
    with_runing_mode->m_cfg_runing_mode = NULL;
    with_runing_mode->m_body_name = NULL;
    with_runing_mode->m_saved_runing_mode = ui_sprite_chipmunk_runing_mode_passive;
    return 0;
}

static void ui_sprite_chipmunk_with_runing_mode_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_runing_mode_t with_runing_mode = (ui_sprite_chipmunk_with_runing_mode_t)ui_sprite_fsm_action_data(fsm_action);

    assert(with_runing_mode->m_body_name == NULL);
    
    if (with_runing_mode->m_cfg_runing_mode) {
        mem_free(module->m_alloc, with_runing_mode->m_cfg_runing_mode);
        with_runing_mode->m_cfg_runing_mode = NULL;
    }

    if (with_runing_mode->m_cfg_body_name) {
        mem_free(module->m_alloc, with_runing_mode->m_cfg_body_name);
        with_runing_mode->m_cfg_body_name = NULL;
    }
}

static int ui_sprite_chipmunk_with_runing_mode_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_runing_mode_t to_with_runing_mode = (ui_sprite_chipmunk_with_runing_mode_t)ui_sprite_fsm_action_data(to);
    ui_sprite_chipmunk_with_runing_mode_t from_with_runing_mode = (ui_sprite_chipmunk_with_runing_mode_t)ui_sprite_fsm_action_data(from);

    if (ui_sprite_chipmunk_with_runing_mode_init(to, ctx)) return -1;

    if (from_with_runing_mode->m_cfg_body_name) {
        to_with_runing_mode->m_cfg_body_name = cpe_str_mem_dup(module->m_alloc, from_with_runing_mode->m_cfg_body_name);
    }

    if (from_with_runing_mode->m_cfg_runing_mode) {
        to_with_runing_mode->m_cfg_runing_mode = cpe_str_mem_dup(module->m_alloc, from_with_runing_mode->m_cfg_runing_mode);
    }

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_chipmunk_with_runing_mode_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_env_t env;
    ui_sprite_world_t world = ui_sprite_fsm_to_world(ui_sprite_fsm_state_fsm(fsm_state));
    ui_sprite_chipmunk_with_runing_mode_t with_runing_mode;
    const char * str_value;
    
    env = ui_sprite_chipmunk_env_find(world);
    if (env == NULL) {
        CPE_ERROR(module->m_em, "%s: create chipmunk-with-runing-mode action: env not exist!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    with_runing_mode = ui_sprite_chipmunk_with_runing_mode_create(fsm_state, name);
    if (with_runing_mode == NULL) {
        CPE_ERROR(module->m_em, "%s: create chipmunk-with-runing-mode action: create fail!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    str_value = cfg_get_string(cfg, "runing-mode", NULL);
    if (str_value == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create chipmunk-with-runing-mode action: runing-mode not configured!",
            ui_sprite_chipmunk_module_name(module), str_value);
        ui_sprite_chipmunk_with_runing_mode_free(with_runing_mode);
        return NULL;
    }
    else {
        with_runing_mode->m_cfg_runing_mode = cpe_str_mem_dup(module->m_alloc, str_value);
        if (with_runing_mode->m_cfg_runing_mode == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create chipmunk-with-runing-mode action: dup runing-mode %s fail!",
                ui_sprite_chipmunk_module_name(module), str_value);
            ui_sprite_chipmunk_with_runing_mode_free(with_runing_mode);            
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "body-mode", NULL))) {
        with_runing_mode->m_cfg_body_name = cpe_str_mem_dup(module->m_alloc, str_value);
        if (with_runing_mode->m_cfg_body_name == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create chipmunk-with-runing-mode action: dup body name %s fail!",
                ui_sprite_chipmunk_module_name(module), str_value);
            ui_sprite_chipmunk_with_runing_mode_free(with_runing_mode);            
            return NULL;
        }
    }
    
    return ui_sprite_fsm_action_from_data(with_runing_mode);
}

    
int ui_sprite_chipmunk_with_runing_mode_regist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_RUNING_MODE_NAME, sizeof(struct ui_sprite_chipmunk_with_runing_mode));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: chipmunk with runing_mode register: meta create fail",
            ui_sprite_chipmunk_module_name  (module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_chipmunk_with_runing_mode_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_chipmunk_with_runing_mode_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_chipmunk_with_runing_mode_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_chipmunk_with_runing_mode_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_chipmunk_with_runing_mode_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_WITH_RUNING_MODE_NAME, ui_sprite_chipmunk_with_runing_mode_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_with_runing_mode_unregist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_RUNING_MODE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CHIPMUNK_WITH_RUNING_MODE_NAME);
}

const char * UI_SPRITE_CHIPMUNK_WITH_RUNING_MODE_NAME = "chipmunk-with-runing-mode";

#ifdef __cplusplus
}
#endif
    
