#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "render/model/ui_data_utils.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_control.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_fsm/ui_sprite_fsm_module.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_render/ui_sprite_render_env.h"
#include "ui/sprite_ui/ui_sprite_ui_utils.h"
#include "ui_sprite_ui_action_entity_follow_control_i.h"

ui_sprite_ui_action_entity_follow_control_t
ui_sprite_ui_action_entity_follow_control_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_UI_ACTION_ENTITY_FOLLOW_CONTROL_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ui_action_entity_follow_control_free(ui_sprite_ui_action_entity_follow_control_t show_ui) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_ui));
}

const char * ui_sprite_ui_action_entity_follow_control_name(ui_sprite_ui_action_entity_follow_control_t entity_follow_control) {
    return entity_follow_control->m_cfg_control;
}

void ui_sprite_ui_action_entity_follow_control_set_name(ui_sprite_ui_action_entity_follow_control_t entity_follow_control, const char * name) {
    if (entity_follow_control->m_cfg_control) {
        mem_free(entity_follow_control->m_module->m_alloc, entity_follow_control->m_cfg_control);
    }

    if (name) {
        entity_follow_control->m_cfg_control = cpe_str_mem_dup_trim(entity_follow_control->m_module->m_alloc, name);
    }
    else {
        entity_follow_control->m_cfg_control = NULL;
    }
}

static int ui_sprite_ui_action_entity_follow_control_update_pos(
    ui_sprite_ui_module_t module, ui_sprite_fsm_action_t fsm_action,
    ui_sprite_ui_action_entity_follow_control_t entity_follow_control, ui_sprite_entity_t entity)
{
    plugin_ui_control_t control;
    ui_vector_2 pos;
    ui_sprite_2d_transform_t transform;
    ui_sprite_render_env_t render_env;

    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): follow control: entity no transform",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    
    control = ui_sprite_ui_find_control_from_action(module, fsm_action, entity_follow_control->m_control);
    if (control == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): follow control: control %s not exist",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), entity_follow_control->m_control);
        return -1;
    }

    
    pos = plugin_ui_control_calc_local_pt_by_policy(control, entity_follow_control->m_cfg_base_policy);
    ui_vector_2_inline_add(&pos, plugin_ui_control_real_pt_abs(control));
    
    render_env = ui_sprite_render_env_find(ui_sprite_entity_world(entity));
    if (render_env) {
        pos = ui_sprite_render_env_screen_to_logic(render_env, &pos);
    }

    ui_sprite_2d_transform_set_origin_pos(transform, pos);
    
    return 0;
}

static int ui_sprite_ui_action_entity_follow_control_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_entity_follow_control_t entity_follow_control = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    
    assert(entity_follow_control->m_control == NULL);

    entity_follow_control->m_control =
        ui_sprite_fsm_action_check_calc_str_dup(module->m_alloc, entity_follow_control->m_cfg_control, fsm_action, NULL, module->m_em);
    if (entity_follow_control->m_control == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): follow control: calc control %s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), entity_follow_control->m_cfg_control);
        return -1;
    }

    if (ui_sprite_ui_action_entity_follow_control_update_pos(module, fsm_action, entity_follow_control, entity) != 0) {
        goto ENTER_FAIL;
    }
    
    ui_sprite_fsm_action_start_update(fsm_action);

    return 0;
    
ENTER_FAIL:
    if (entity_follow_control->m_control) {
        mem_free(module->m_alloc, entity_follow_control->m_control);
        entity_follow_control->m_control = NULL;
    }

    return -1;
}

static void ui_sprite_ui_action_entity_follow_control_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_entity_follow_control_t entity_follow_control = ui_sprite_fsm_action_data(fsm_action);

    assert(entity_follow_control->m_control);

    if (entity_follow_control->m_control) {
        mem_free(module->m_alloc, entity_follow_control->m_control);
        entity_follow_control->m_control = NULL;
    }
}

static void ui_sprite_ui_action_entity_follow_control_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_entity_follow_control_t entity_follow_control = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    
    assert(entity_follow_control->m_control);

    if (ui_sprite_ui_action_entity_follow_control_update_pos(module, fsm_action, entity_follow_control, entity) != 0) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

int ui_sprite_ui_action_entity_follow_control_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_action_entity_follow_control_t entity_follow_control = ui_sprite_fsm_action_data(fsm_action);

    entity_follow_control->m_module = ctx;
    entity_follow_control->m_cfg_control = NULL;
    entity_follow_control->m_cfg_base_policy = ui_pos_policy_center;
    entity_follow_control->m_control = NULL;
    
    return 0;
}

int ui_sprite_ui_action_entity_follow_control_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_entity_follow_control_t to_entity_follow_control = ui_sprite_fsm_action_data(to);
    ui_sprite_ui_action_entity_follow_control_t from_entity_follow_control = ui_sprite_fsm_action_data(from);
    
    ui_sprite_ui_action_entity_follow_control_init(to, ctx);

    if (from_entity_follow_control->m_cfg_control) {
        to_entity_follow_control->m_cfg_control = cpe_str_mem_dup(module->m_alloc, from_entity_follow_control->m_cfg_control);
    }

    to_entity_follow_control->m_cfg_base_policy = from_entity_follow_control->m_cfg_base_policy;
    
    return 0;
}

void ui_sprite_ui_action_entity_follow_control_fini(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_entity_follow_control_t entity_follow_control = ui_sprite_fsm_action_data(fsm_action);

    assert(entity_follow_control->m_control == NULL);
    
    if (entity_follow_control->m_cfg_control) {
        mem_free(module->m_alloc, entity_follow_control->m_cfg_control);
        entity_follow_control->m_cfg_control = NULL;
    }
}

static ui_sprite_fsm_action_t ui_sprite_ui_action_entity_follow_control_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_entity_follow_control_t entity_follow_control = ui_sprite_ui_action_entity_follow_control_create(fsm_state, name);
    const char * str_value;

    if (entity_follow_control == NULL) {
        CPE_ERROR(module->m_em, "%s: create entity_follow_control action: create fail!", ui_sprite_ui_module_name(module));
        return NULL;
    }

    str_value = cfg_get_string(cfg, "control", "");
    if (str_value == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create entity follow control: control not configured!",
            ui_sprite_ui_module_name(module));
        ui_sprite_ui_action_entity_follow_control_free(entity_follow_control);
        return NULL;
    }
    else {
        ui_sprite_ui_action_entity_follow_control_set_name(entity_follow_control, str_value);
    }
    
    if ((str_value = cfg_get_string(cfg, "base-policy", NULL))) {
        entity_follow_control->m_cfg_base_policy = ui_pos_policy_from_str(str_value);
    }

    return ui_sprite_fsm_action_from_data(entity_follow_control);
}

int ui_sprite_ui_action_entity_follow_control_regist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_UI_ACTION_ENTITY_FOLLOW_CONTROL_NAME, sizeof(struct ui_sprite_ui_action_entity_follow_control));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_ui_module_name(module), UI_SPRITE_UI_ACTION_ENTITY_FOLLOW_CONTROL_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ui_action_entity_follow_control_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ui_action_entity_follow_control_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ui_action_entity_follow_control_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ui_action_entity_follow_control_exit, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_ui_action_entity_follow_control_update, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ui_action_entity_follow_control_fini, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(
                module->m_loader, UI_SPRITE_UI_ACTION_ENTITY_FOLLOW_CONTROL_NAME, ui_sprite_ui_action_entity_follow_control_load, module) != 0)
        {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_ui_action_entity_follow_control_unregist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_UI_ACTION_ENTITY_FOLLOW_CONTROL_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_ui_module_name(module), UI_SPRITE_UI_ACTION_ENTITY_FOLLOW_CONTROL_NAME);
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_UI_ACTION_ENTITY_FOLLOW_CONTROL_NAME);
    }
}

const char * UI_SPRITE_UI_ACTION_ENTITY_FOLLOW_CONTROL_NAME = "entity-follow-control";
