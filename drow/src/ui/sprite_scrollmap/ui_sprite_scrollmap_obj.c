#include <assert.h>
#include <stdio.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "plugin/scrollmap/plugin_scrollmap_obj.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui_sprite_scrollmap_obj_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_scrollmap_obj_t ui_sprite_scrollmap_obj_create(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_create(entity, UI_SPRITE_SCROLLMAP_OBJ_NAME);
    return component ? (ui_sprite_scrollmap_obj_t)ui_sprite_component_data(component) : NULL;
};

ui_sprite_scrollmap_obj_t ui_sprite_scrollmap_obj_find(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_SCROLLMAP_OBJ_NAME);
    return component ? (ui_sprite_scrollmap_obj_t)ui_sprite_component_data(component) : NULL;
};

void ui_sprite_scrollmap_obj_free(ui_sprite_scrollmap_obj_t scrollmap_obj) {
    ui_sprite_component_t component = ui_sprite_component_from_data(scrollmap_obj);
    if (component) {
        ui_sprite_component_free(component);
    }
}
    
static int ui_sprite_scrollmap_obj_do_init(ui_sprite_scrollmap_module_t module, ui_sprite_scrollmap_obj_t obj, ui_sprite_entity_t entity) {
    ui_sprite_scrollmap_env_t env = ui_sprite_scrollmap_env_find(ui_sprite_entity_world(entity));

    if (env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): scrollmap obj init: no chipmunt_env!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    obj->m_env = env;
    obj->m_obj = NULL;
    obj->m_is_suspend = 0;
    
    return 0;
}

static int ui_sprite_scrollmap_obj_init(ui_sprite_component_t component, void * ctx) {
    ui_sprite_scrollmap_module_t module = (ui_sprite_scrollmap_module_t)ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_scrollmap_obj_t scrollmap_obj = (ui_sprite_scrollmap_obj_t)ui_sprite_component_data(component);

    if (ui_sprite_scrollmap_obj_do_init(module, scrollmap_obj, entity) != 0) return -1;

    return 0;
}

static void ui_sprite_scrollmap_obj_fini(ui_sprite_component_t component, void * ctx) {
    ui_sprite_scrollmap_obj_t scrollmap_obj = (ui_sprite_scrollmap_obj_t)ui_sprite_component_data(component);

    if (scrollmap_obj->m_obj) {
        struct ui_sprite_scrollmap_obj_stub * stub = (struct ui_sprite_scrollmap_obj_stub *)plugin_scrollmap_obj_data(scrollmap_obj->m_obj);
        stub->m_sprite_obj = NULL;
        plugin_scrollmap_obj_free(scrollmap_obj->m_obj);
        scrollmap_obj->m_obj = NULL;
    }
}

static int ui_sprite_scrollmap_obj_copy(ui_sprite_component_t to, ui_sprite_component_t from, void * ctx) {
    ui_sprite_scrollmap_module_t module = (ui_sprite_scrollmap_module_t)ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(to);
    ui_sprite_scrollmap_obj_t to_scrollmap_obj = (ui_sprite_scrollmap_obj_t)ui_sprite_component_data(to);
    ui_sprite_scrollmap_obj_t from_scrollmap_obj = (ui_sprite_scrollmap_obj_t)ui_sprite_component_data(from);

    if (ui_sprite_scrollmap_obj_do_init(module, to_scrollmap_obj, entity) != 0) return -1;

    to_scrollmap_obj->m_is_suspend = from_scrollmap_obj->m_is_suspend;

    return 0;
}

static void ui_sprite_scrollmap_obj_exit(ui_sprite_component_t component, void * ctx) {
}

static int ui_sprite_scrollmap_obj_enter(ui_sprite_component_t component, void * ctx) {
    return 0;
}

uint8_t ui_sprite_scrollmap_obj_is_move_suspend(ui_sprite_scrollmap_obj_t scrollmap_obj) {
    return scrollmap_obj->m_is_suspend;
}
    
void ui_sprite_scrollmap_obj_set_move_suspend(ui_sprite_scrollmap_obj_t scrollmap_obj, uint8_t is_suspend) {
    scrollmap_obj->m_is_suspend = is_suspend;
    if (scrollmap_obj->m_obj) {
        plugin_scrollmap_obj_set_move_suspend(scrollmap_obj->m_obj, is_suspend);
    }
}

static int ui_sprite_scrollmap_obj_load(void * ctx, ui_sprite_component_t comp, cfg_t cfg) {
    ui_sprite_scrollmap_obj_t scrollmap_obj = (ui_sprite_scrollmap_obj_t)ui_sprite_component_data(comp);
    scrollmap_obj->m_is_suspend = cfg_get_uint8(cfg, "suspend", scrollmap_obj->m_is_suspend);
    return 0;
}
    
int ui_sprite_scrollmap_obj_regist(ui_sprite_scrollmap_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_create(
        module->m_repo,
        UI_SPRITE_SCROLLMAP_OBJ_NAME, sizeof(struct ui_sprite_scrollmap_obj));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_scrollmap_module_name(module), UI_SPRITE_SCROLLMAP_OBJ_NAME);
        return -1;
    }

    ui_sprite_component_meta_set_init_fun(meta, ui_sprite_scrollmap_obj_init, module);
    ui_sprite_component_meta_set_copy_fun(meta, ui_sprite_scrollmap_obj_copy, module);
    ui_sprite_component_meta_set_enter_fun(meta, ui_sprite_scrollmap_obj_enter, module);
    ui_sprite_component_meta_set_exit_fun(meta, ui_sprite_scrollmap_obj_exit, module);
    ui_sprite_component_meta_set_free_fun(meta, ui_sprite_scrollmap_obj_fini, module);

    if (ui_sprite_cfg_loader_add_comp_loader(
            module->m_loader, UI_SPRITE_SCROLLMAP_OBJ_NAME, ui_sprite_scrollmap_obj_load, module)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: %s register: register loader fail",
            ui_sprite_scrollmap_module_name(module), UI_SPRITE_SCROLLMAP_OBJ_NAME);
        ui_sprite_component_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_scrollmap_obj_unregist(ui_sprite_scrollmap_module_t module) {
    ui_sprite_component_meta_t meta;

    if ((meta = ui_sprite_component_meta_find(module->m_repo, UI_SPRITE_SCROLLMAP_OBJ_NAME))) {
        ui_sprite_component_meta_free(meta);
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_scrollmap_module_name(module), UI_SPRITE_SCROLLMAP_OBJ_NAME);
    }

    if (ui_sprite_cfg_loader_remove_comp_loader(module->m_loader, UI_SPRITE_SCROLLMAP_OBJ_NAME) != 0) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: loader remove fail",
            ui_sprite_scrollmap_module_name(module), UI_SPRITE_SCROLLMAP_OBJ_NAME);
    }
}

const char * UI_SPRITE_SCROLLMAP_OBJ_NAME = "ScrollMapObj";

#ifdef __cplusplus
}
#endif

