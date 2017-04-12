#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_module.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_render_action_adj_priority_i.h"
#include "ui_sprite_render_group_i.h"

ui_sprite_render_action_adj_priority_t
ui_sprite_render_action_adj_priority_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_RENDER_ACTION_ADJ_PRIORITY_TYPE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_render_action_adj_priority_free(ui_sprite_render_action_adj_priority_t show_anim) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_anim));
}

const char * ui_sprite_render_action_adj_priority_group(ui_sprite_render_action_adj_priority_t action_adj_priority) {
    return action_adj_priority->m_cfg_group;
}

void ui_sprite_render_action_adj_priority_set_group(ui_sprite_render_action_adj_priority_t action_adj_priority, const char * group) {
    ui_sprite_render_module_t module = action_adj_priority->m_module;
    
    if (action_adj_priority->m_cfg_group) {
        mem_free(module->m_alloc, action_adj_priority->m_cfg_group);
    }

    if (group) {
        action_adj_priority->m_cfg_group = cpe_str_mem_dup_trim(module->m_alloc, group);
    }
    else {
        action_adj_priority->m_cfg_group = NULL;
    }
}

const char * ui_sprite_render_action_adj_priority_action_adj_priority(ui_sprite_render_action_adj_priority_t action_adj_priority) {
    return action_adj_priority->m_cfg_action_adj_priority;
}

void ui_sprite_render_action_adj_priority_set_action_adj_priority(ui_sprite_render_action_adj_priority_t action_adj_priority, const char * str_action_adj_priority) {
    ui_sprite_render_module_t module = action_adj_priority->m_module;
    
    if (action_adj_priority->m_cfg_action_adj_priority) {
        mem_free(module->m_alloc, action_adj_priority->m_cfg_action_adj_priority);
    }

    if (str_action_adj_priority) {
        action_adj_priority->m_cfg_action_adj_priority = cpe_str_mem_dup_trim(module->m_alloc, str_action_adj_priority);
    }
    else {
        action_adj_priority->m_cfg_action_adj_priority = NULL;
    }
}

static void ui_sprite_render_action_adj_priority_on_attr_updated(void * ctx) {
    ui_sprite_render_action_adj_priority_t action_adj_priority = (ui_sprite_render_action_adj_priority_t)ctx;
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(action_adj_priority);
    float adj_value;
    
    if (ui_sprite_fsm_action_check_calc_float(
            &adj_value,
            action_adj_priority->m_cfg_action_adj_priority,
            fsm_action, NULL,
            action_adj_priority->m_module->m_em) != 0)
    {
        ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
        CPE_ERROR(
            action_adj_priority->m_module->m_em,
            "entity %d(%s): render adj priority: calc adj-priority from %s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), action_adj_priority->m_cfg_action_adj_priority);
        return;
    }

    ui_sprite_render_group_set_adj_render_priority(action_adj_priority->m_group, action_adj_priority->m_base_priority + adj_value);
}

int ui_sprite_render_action_adj_priority_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = (ui_sprite_render_module_t)ctx;
    ui_sprite_render_action_adj_priority_t action_adj_priority = (ui_sprite_render_action_adj_priority_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_render_sch_t anim_sch = ui_sprite_render_sch_find(entity);
    const char * group_name;
    
    if (anim_sch == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): render adj priority: no anim_sch",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    assert(action_adj_priority->m_group == NULL);

    if (action_adj_priority->m_cfg_action_adj_priority == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): render adj priority: adj-priority not configured",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_ERROR;
    }

    if (action_adj_priority->m_cfg_action_adj_priority[0] == ':') {
        if (ui_sprite_fsm_action_add_attr_monitor_by_def(
                fsm_action,
                action_adj_priority->m_cfg_action_adj_priority + 1,
                ui_sprite_render_action_adj_priority_on_attr_updated,
                action_adj_priority))
        {
            CPE_ERROR(
                module->m_em, "entity %d(%s): render adj priority: add attr monitor by defs %s fail",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), action_adj_priority->m_cfg_group);
            goto ENTER_ERROR;
        }
    }    
    
    if (action_adj_priority->m_cfg_group) {
        group_name = ui_sprite_fsm_action_check_calc_str(gd_app_tmp_buffer(module->m_app), action_adj_priority->m_cfg_group, fsm_action, NULL, NULL);
        if (group_name == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): render adj priority: calc group from %s fail",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), action_adj_priority->m_cfg_group);
            goto ENTER_ERROR;
        }
    }
    else {
        group_name = "";
    }
        
    action_adj_priority->m_group = ui_sprite_render_group_find_by_name(anim_sch, group_name);
    if (action_adj_priority->m_group == NULL) {
        CPE_ERROR(
            action_adj_priority->m_module->m_em,
            "entity %d(%s): render adj priority: group %s not exist",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            group_name);
        return -1;
    }
    
    action_adj_priority->m_base_priority = ui_sprite_render_group_adj_render_priority(action_adj_priority->m_group);

    ui_sprite_render_action_adj_priority_on_attr_updated(action_adj_priority);
    
    return 0;

ENTER_ERROR:
    return -1;
}

void ui_sprite_render_action_adj_priority_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_action_adj_priority_t action_adj_priority = ui_sprite_fsm_action_data(fsm_action);
    
    assert(action_adj_priority->m_group);

    ui_sprite_render_group_set_adj_render_priority(action_adj_priority->m_group, action_adj_priority->m_base_priority);
    action_adj_priority->m_group = NULL;
    action_adj_priority->m_base_priority = 0.0f;
}

int ui_sprite_render_action_adj_priority_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_action_adj_priority_t action_adj_priority = ui_sprite_fsm_action_data(fsm_action);

    action_adj_priority->m_module = ctx;
    action_adj_priority->m_cfg_group = NULL;
    action_adj_priority->m_cfg_action_adj_priority = NULL;
    action_adj_priority->m_group = NULL;
    action_adj_priority->m_base_priority = 0.0f;

    return 0;
}

int ui_sprite_render_action_adj_priority_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_action_adj_priority_t to_action_adj_priority = ui_sprite_fsm_action_data(to);
    ui_sprite_render_action_adj_priority_t from_action_adj_priority = ui_sprite_fsm_action_data(from);

    if (ui_sprite_render_action_adj_priority_init(to, ctx) != 0) return -1;

    if (from_action_adj_priority->m_cfg_action_adj_priority) {
        to_action_adj_priority->m_cfg_action_adj_priority = cpe_str_mem_dup(module->m_alloc, from_action_adj_priority->m_cfg_action_adj_priority);
    }

    if (from_action_adj_priority->m_cfg_group) {
        to_action_adj_priority->m_cfg_group = cpe_str_mem_dup(module->m_alloc, from_action_adj_priority->m_cfg_group);
    }
    
    return 0;
}

void ui_sprite_render_action_adj_priority_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_action_adj_priority_t action_adj_priority = ui_sprite_fsm_action_data(fsm_action);

    assert(action_adj_priority->m_group == NULL);
    assert(action_adj_priority->m_base_priority == 0.0f);

    if (action_adj_priority->m_cfg_action_adj_priority) {
        mem_free(module->m_alloc, action_adj_priority->m_cfg_action_adj_priority);
        action_adj_priority->m_cfg_action_adj_priority = NULL;
    }

    if (action_adj_priority->m_cfg_group) {
        mem_free(module->m_alloc, action_adj_priority->m_cfg_group);
        action_adj_priority->m_cfg_group = NULL;
    }
}

static ui_sprite_fsm_action_t ui_sprite_render_action_adj_priority_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_action_adj_priority_t action_adj_priority = ui_sprite_render_action_adj_priority_create(fsm_state, name);
    const char * value;

    if (action_adj_priority == NULL) {
        CPE_ERROR(module->m_em, "%s: create show-animation action: create fail!", ui_sprite_render_module_name(module));
        return NULL;
    }

    value = cfg_get_string(cfg, "adj-priority", NULL);
    if (value == NULL) {
        CPE_ERROR(module->m_em, "%s: create show-animation action: adj-priority not configured!", ui_sprite_render_module_name(module));
        ui_sprite_render_action_adj_priority_free(action_adj_priority);
        return NULL;
    }
    ui_sprite_render_action_adj_priority_set_action_adj_priority(action_adj_priority, value);

    ui_sprite_render_action_adj_priority_set_group(action_adj_priority, cfg_get_string(cfg, "group", NULL));

    return ui_sprite_fsm_action_from_data(action_adj_priority);
}

int ui_sprite_render_action_adj_priority_regist(ui_sprite_render_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_RENDER_ACTION_ADJ_PRIORITY_TYPE_NAME, sizeof(struct ui_sprite_render_action_adj_priority));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_render_module_name(module), UI_SPRITE_RENDER_ACTION_ADJ_PRIORITY_TYPE_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_render_action_adj_priority_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_render_action_adj_priority_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_render_action_adj_priority_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_render_action_adj_priority_exit, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_render_action_adj_priority_clear, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(
                module->m_loader, UI_SPRITE_RENDER_ACTION_ADJ_PRIORITY_TYPE_NAME, ui_sprite_render_action_adj_priority_load, module) != 0)
        {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_render_action_adj_priority_unregist(ui_sprite_render_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_RENDER_ACTION_ADJ_PRIORITY_TYPE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_render_module_name(module), UI_SPRITE_RENDER_ACTION_ADJ_PRIORITY_TYPE_NAME);
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_RENDER_ACTION_ADJ_PRIORITY_TYPE_NAME);
    }
}

const char * UI_SPRITE_RENDER_ACTION_ADJ_PRIORITY_TYPE_NAME = "render-adj-priority";
