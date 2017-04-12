#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "plugin/particle/plugin_particle_obj_plugin_data.h"
#include "plugin/particle/plugin_particle_obj_particle.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui_sprite_particle_controled_obj_i.h"
#include "ui_sprite_particle_gen_entity_i.h"
#include "ui_sprite_particle_gen_entity_slot_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_particle_controled_obj_t ui_sprite_particle_controled_obj_create(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_create(entity, UI_SPRITE_PARTICLE_CONTROLED_OBJ_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

ui_sprite_particle_controled_obj_t ui_sprite_particle_controled_obj_find(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_PARTICLE_CONTROLED_OBJ_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

void ui_sprite_particle_controled_obj_set_slot(ui_sprite_particle_controled_obj_t obj, ui_sprite_particle_gen_entity_slot_t slot) {
    if (obj->m_slot == slot) return;
    
    if (obj->m_slot) {
        if (obj->m_remove_particle) {
            plugin_particle_obj_particle_free(
                plugin_particle_obj_plugin_data_particle(
                    plugin_particle_obj_plugin_data_from_data(obj->m_slot)));
        }
        else {
            obj->m_slot->m_controled_obj = NULL;
        }
    }

    obj->m_slot = slot;

    if (obj->m_slot) {
        obj->m_slot->m_controled_obj = obj;
    }
}

uint8_t ui_sprite_particle_controled_obj_is_binding(ui_sprite_particle_controled_obj_t obj) {
    return obj->m_is_binding;
}
    
void ui_sprite_particle_controled_obj_set_binding(ui_sprite_particle_controled_obj_t obj, uint8_t is_binding) {
    obj->m_is_binding = is_binding;
}

void ui_sprite_particle_controled_obj_free(ui_sprite_particle_controled_obj_t particle_controled_obj) {
    ui_sprite_component_t component = ui_sprite_component_from_data(particle_controled_obj);
    if (component) {
        ui_sprite_component_free(component);
    }
}
    
static int ui_sprite_particle_controled_obj_do_init(ui_sprite_particle_module_t module, ui_sprite_particle_controled_obj_t obj) {
    obj->m_module = module;
    obj->m_slot = NULL;
    obj->m_is_binding = 1;
    obj->m_accept_scale = 1;
    obj->m_accept_angle = 1;
    obj->m_remove_particle = 1;
    return 0;
}

static int ui_sprite_particle_controled_obj_init(ui_sprite_component_t component, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_controled_obj_t obj = ui_sprite_component_data(component);

    if (ui_sprite_particle_controled_obj_do_init(module, obj) != 0) return -1;
    
    return 0;
}

static void ui_sprite_particle_controled_obj_fini(ui_sprite_component_t component, void * ctx) {
    ui_sprite_particle_controled_obj_t obj = ui_sprite_component_data(component);
    if (obj->m_slot) {
        obj->m_slot->m_controled_obj = NULL;
        obj->m_slot = NULL;
    }
}

static int ui_sprite_particle_controled_obj_copy(ui_sprite_component_t to, ui_sprite_component_t from, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_controled_obj_t to_obj = ui_sprite_component_data(to);
    ui_sprite_particle_controled_obj_t from_obj = ui_sprite_component_data(from);

    if (ui_sprite_particle_controled_obj_do_init(module, to_obj) != 0) return -1;

    to_obj->m_is_binding = from_obj->m_is_binding;
    to_obj->m_accept_angle = from_obj->m_accept_angle;
    to_obj->m_accept_scale = from_obj->m_accept_scale;

    return 0;
}

static void ui_sprite_particle_controled_obj_exit(ui_sprite_component_t component, void * ctx) {
    ui_sprite_particle_controled_obj_t obj = ui_sprite_component_data(component);
    if (obj->m_slot) {
        if (obj->m_remove_particle) {
            obj->m_slot->m_controled_obj = NULL;
            plugin_particle_obj_particle_free(
                plugin_particle_obj_plugin_data_particle(
                    plugin_particle_obj_plugin_data_from_data(obj->m_slot)));
        }
        else {
            obj->m_slot->m_controled_obj = NULL;
        }
    }
}

static int ui_sprite_particle_controled_obj_enter(ui_sprite_component_t component, void * ctx) {
    return 0;
}

static int ui_sprite_particle_controled_obj_load(void * ctx, ui_sprite_component_t comp, cfg_t cfg) {
    ui_sprite_particle_controled_obj_t obj = ui_sprite_component_data(comp);
    obj->m_is_binding = cfg_get_uint8(cfg, "binding", obj->m_is_binding);
    return 0;
}
    
int ui_sprite_particle_controled_obj_regist(ui_sprite_particle_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_create(
        module->m_repo,
        UI_SPRITE_PARTICLE_CONTROLED_OBJ_NAME, sizeof(struct ui_sprite_particle_controled_obj));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_particle_module_name(module), UI_SPRITE_PARTICLE_CONTROLED_OBJ_NAME);
        return -1;
    }

    ui_sprite_component_meta_set_init_fun(meta, ui_sprite_particle_controled_obj_init, module);
    ui_sprite_component_meta_set_copy_fun(meta, ui_sprite_particle_controled_obj_copy, module);
    ui_sprite_component_meta_set_enter_fun(meta, ui_sprite_particle_controled_obj_enter, module);
    ui_sprite_component_meta_set_exit_fun(meta, ui_sprite_particle_controled_obj_exit, module);
    ui_sprite_component_meta_set_free_fun(meta, ui_sprite_particle_controled_obj_fini, module);

    if (ui_sprite_cfg_loader_add_comp_loader(
            module->m_loader, UI_SPRITE_PARTICLE_CONTROLED_OBJ_NAME, ui_sprite_particle_controled_obj_load, module)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: %s register: register loader fail",
            ui_sprite_particle_module_name(module), UI_SPRITE_PARTICLE_CONTROLED_OBJ_NAME);
        ui_sprite_component_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_particle_controled_obj_unregist(ui_sprite_particle_module_t module) {
    ui_sprite_component_meta_t meta;

    if ((meta = ui_sprite_component_meta_find(module->m_repo, UI_SPRITE_PARTICLE_CONTROLED_OBJ_NAME))) {
        ui_sprite_component_meta_free(meta);
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_particle_module_name(module), UI_SPRITE_PARTICLE_CONTROLED_OBJ_NAME);
    }

    if (ui_sprite_cfg_loader_remove_comp_loader(module->m_loader, UI_SPRITE_PARTICLE_CONTROLED_OBJ_NAME) != 0) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: loader remove fail",
            ui_sprite_particle_module_name(module), UI_SPRITE_PARTICLE_CONTROLED_OBJ_NAME);
    }
}

const char * UI_SPRITE_PARTICLE_CONTROLED_OBJ_NAME = "ParticleControledObj";

#ifdef __cplusplus
}
#endif

