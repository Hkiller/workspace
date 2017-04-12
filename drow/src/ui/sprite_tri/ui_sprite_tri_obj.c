#include <assert.h>
#include <stdio.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "gd/app/app_context.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui_sprite_tri_obj_i.h"
#include "ui_sprite_tri_rule_i.h"
#include "ui_sprite_tri_condition_i.h"
#include "ui_sprite_tri_action_i.h"

ui_sprite_tri_obj_t ui_sprite_tri_obj_create(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_create(entity, UI_SPRITE_TRI_OBJ_NAME);
    return component ? (ui_sprite_tri_obj_t)ui_sprite_component_data(component) : NULL;
};

ui_sprite_tri_obj_t ui_sprite_tri_obj_find(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_TRI_OBJ_NAME);
    return component ? (ui_sprite_tri_obj_t)ui_sprite_component_data(component) : NULL;
};

void ui_sprite_tri_obj_free(ui_sprite_tri_obj_t tri_obj) {
    ui_sprite_component_t component = ui_sprite_component_from_data(tri_obj);
    if (component) {
        ui_sprite_component_free(component);
    }
}

ui_sprite_tri_module_t ui_sprite_tri_obj_module(ui_sprite_tri_obj_t tri_obj) {
    return tri_obj->m_module;
}

static int ui_sprite_tri_obj_enter(ui_sprite_component_t component, void * ctx) {
    ui_sprite_tri_obj_t tri_obj = (ui_sprite_tri_obj_t)ui_sprite_component_data(component);

    if (!TAILQ_EMPTY(&tri_obj->m_rules)) {
        ui_sprite_component_start_update(component);
    }
    
    return 0;
}

static int ui_sprite_tri_obj_do_init(ui_sprite_tri_module_t module, ui_sprite_tri_obj_t obj, ui_sprite_entity_t entity) {
    obj->m_module = module;
    TAILQ_INIT(&obj->m_rules);
    return 0;
}

static int ui_sprite_tri_obj_init(ui_sprite_component_t component, void * ctx) {
    ui_sprite_tri_module_t module = (ui_sprite_tri_module_t)ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_tri_obj_t tri_obj = (ui_sprite_tri_obj_t)ui_sprite_component_data(component);

    if (ui_sprite_tri_obj_do_init(module, tri_obj, entity) != 0) return -1;

    return 0;
}

static void ui_sprite_tri_obj_fini(ui_sprite_component_t component, void * ctx) {
    ui_sprite_tri_obj_t tri_obj = (ui_sprite_tri_obj_t)ui_sprite_component_data(component);

    while(!TAILQ_EMPTY(&tri_obj->m_rules)) {
        ui_sprite_tri_rule_free(TAILQ_FIRST(&tri_obj->m_rules));
    }
}

static int ui_sprite_tri_obj_copy(ui_sprite_component_t to, ui_sprite_component_t from, void * ctx) {
    ui_sprite_tri_module_t module = (ui_sprite_tri_module_t)ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(to);
    ui_sprite_tri_obj_t from_tri_obj = (ui_sprite_tri_obj_t)ui_sprite_component_data(from);
    ui_sprite_tri_obj_t to_tri_obj = (ui_sprite_tri_obj_t)ui_sprite_component_data(to);
    ui_sprite_tri_rule_t from_rule;

    if (ui_sprite_tri_obj_do_init(module, to_tri_obj, entity) != 0) return -1;

    TAILQ_FOREACH(from_rule, &from_tri_obj->m_rules, m_next) {
        if (ui_sprite_tri_rule_clone(to_tri_obj, from_rule) == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): tri obj copy: clone rule fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            ui_sprite_tri_obj_fini(to, ctx);
            return -1;
        }
    }
    
    return 0;
}

static void ui_sprite_tri_obj_exit(ui_sprite_component_t component, void * ctx) {
    //ui_sprite_tri_obj_t tri_obj = (ui_sprite_tri_obj_t)ui_sprite_component_data(component);
    //ui_sprite_entity_t entity = ui_sprite_component_entity(component);
}

static void ui_sprite_tri_obj_update(ui_sprite_component_t component, void * ctx, float delta) {
    ui_sprite_tri_obj_t tri_obj = (ui_sprite_tri_obj_t)ui_sprite_component_data(component);
    ui_sprite_tri_rule_t rule, next_rule;

    for(rule = TAILQ_FIRST(&tri_obj->m_rules); rule; rule = next_rule) {
        uint8_t check_r;

        next_rule = TAILQ_NEXT(rule, m_next);

        if (!rule->m_active) continue;
        if (!TAILQ_EMPTY(&rule->m_triggers)) continue;
        if (rule->m_condition == NULL) continue;
        if (ui_sprite_tri_condition_check(rule->m_condition, &check_r) != 0) continue;

        ui_sprite_tri_rule_sync_state(rule, check_r);
    }
    
    if (TAILQ_EMPTY(&tri_obj->m_rules)) {
        ui_sprite_component_stop_update(component);
    }
}

static int ui_sprite_tri_obj_load(void * ctx, ui_sprite_component_t comp, cfg_t cfg) {
    return 0;
}

int ui_sprite_tri_obj_regist(ui_sprite_tri_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_create(
        module->m_repo,
        UI_SPRITE_TRI_OBJ_NAME, sizeof(struct ui_sprite_tri_obj));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_tri_module_name(module), UI_SPRITE_TRI_OBJ_NAME);
        return -1;
    }

    ui_sprite_component_meta_set_init_fun(meta, ui_sprite_tri_obj_init, module);
    ui_sprite_component_meta_set_copy_fun(meta, ui_sprite_tri_obj_copy, module);
    ui_sprite_component_meta_set_enter_fun(meta, ui_sprite_tri_obj_enter, module);
    ui_sprite_component_meta_set_exit_fun(meta, ui_sprite_tri_obj_exit, module);
    ui_sprite_component_meta_set_free_fun(meta, ui_sprite_tri_obj_fini, module);
    ui_sprite_component_meta_set_update_fun(meta, ui_sprite_tri_obj_update, module);

    if (ui_sprite_cfg_loader_add_comp_loader(
            module->m_loader, UI_SPRITE_TRI_OBJ_NAME, ui_sprite_tri_obj_load, module)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: %s register: register loader fail",
            ui_sprite_tri_module_name(module), UI_SPRITE_TRI_OBJ_NAME);
        ui_sprite_component_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_tri_obj_unregist(ui_sprite_tri_module_t module) {
    ui_sprite_component_meta_t meta;

    if ((meta = ui_sprite_component_meta_find(module->m_repo, UI_SPRITE_TRI_OBJ_NAME))) {
        ui_sprite_component_meta_free(meta);
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_tri_module_name(module), UI_SPRITE_TRI_OBJ_NAME);
    }

    if (ui_sprite_cfg_loader_remove_comp_loader(module->m_loader, UI_SPRITE_TRI_OBJ_NAME) != 0) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: loader remove fail",
            ui_sprite_tri_module_name(module), UI_SPRITE_TRI_OBJ_NAME);
    }
}

const char * UI_SPRITE_TRI_OBJ_NAME = "TriObj";
