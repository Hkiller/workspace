#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/utils/string_utils.h"
#include "ui_sprite_chipmunk_monitor_i.h"
#include "ui_sprite_chipmunk_monitor_binding_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"

void ui_sprite_chipmunk_monitor_init(ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_monitor_t monitor) {
    bzero(monitor, sizeof(*monitor));
    TAILQ_INIT(&monitor->m_bindings);
}

void ui_sprite_chipmunk_monitor_fini(ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_monitor_t monitor) {
    assert(TAILQ_EMPTY(&monitor->m_bindings));

    if (monitor->m_bodies) {
        mem_free(module->m_alloc, monitor->m_bodies);
        monitor->m_bodies = NULL;
    }
}

int ui_sprite_chipmunk_monitor_copy(
    ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_monitor_t to_monitor, ui_sprite_chipmunk_monitor_t from_monitor)
{
    assert(to_monitor->m_obj == NULL);

    if (to_monitor->m_bodies) {
        mem_free(module->m_alloc, to_monitor->m_bodies);
        to_monitor->m_bodies = NULL;
    }

    if (from_monitor->m_bodies) {
        to_monitor->m_bodies = cpe_str_mem_dup(module->m_alloc, from_monitor->m_bodies);
        if (to_monitor->m_bodies == NULL) return -1;
    }

    to_monitor->m_collision_category = from_monitor->m_collision_category;
    to_monitor->m_collision_mask = from_monitor->m_collision_mask;

    return 0;
}

int ui_sprite_chipmunk_monitor_enter(
    ui_sprite_chipmunk_obj_t obj, ui_sprite_chipmunk_monitor_t monitor)
{
    ui_sprite_chipmunk_obj_body_t body;

    assert(monitor->m_obj == NULL);

    monitor->m_obj = obj;
    TAILQ_INSERT_TAIL(&obj->m_monitors, monitor, m_next_for_obj);

    if (monitor->m_bodies == NULL) {
        TAILQ_FOREACH(body, &obj->m_bodies, m_next_for_obj) {
            if (ui_sprite_chipmunk_monitor_binding_create(body, monitor) == NULL) return -1;
        }
    }
    else {
        TAILQ_FOREACH(body, &obj->m_bodies, m_next_for_obj) {
            if (cpe_str_is_in_list(body->m_name, monitor->m_bodies, ',')) {
                if (ui_sprite_chipmunk_monitor_binding_create(body, monitor) == NULL) return -1;
            }
        }
    }

    return 0;
}

void ui_sprite_chipmunk_monitor_check(ui_sprite_chipmunk_monitor_t monitor) {
    /* assert(monitor->m_obj); */

    /* if (monitor->m_bodies == NULL) { */
    /*     TAILQ_FOREACH(body, &obj->m_bodies, m_next_for_obj) { */
    /*         if (ui_sprite_chipmunk_monitor_binding_create(body, monitor) == NULL) return -1; */
    /*     } */
    /* } */
    /* else { */
    /*     TAILQ_FOREACH(body, &obj->m_bodies, m_next_for_obj) { */
    /*         if (cpe_str_is_in_list(body->m_name, monitor->m_bodies, ',')) { */
    /*             if (ui_sprite_chipmunk_monitor_binding_create(body, monitor) == NULL) return -1; */
    /*         } */
    /*     } */
    /* } */
}

void ui_sprite_chipmunk_monitor_exit(ui_sprite_chipmunk_monitor_t monitor) {
    assert(monitor->m_obj);

    while(!TAILQ_EMPTY(&monitor->m_bindings)) {
        ui_sprite_chipmunk_monitor_binding_t binding = TAILQ_FIRST(&monitor->m_bindings);
        //ui_sprite_chipmunk_obj_body_t body = binding->m_body;

        ui_sprite_chipmunk_monitor_binding_free(binding);
    }

    TAILQ_REMOVE(&monitor->m_obj->m_monitors, monitor, m_next_for_obj);
    monitor->m_obj = NULL;
}

int ui_sprite_chipmunk_monitor_load(ui_sprite_chipmunk_env_t env, ui_sprite_chipmunk_monitor_t monitor, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = env->m_module;
    const char * v;
    const char * str_value;

    if ((str_value = cfg_get_string(cfg, "mask", NULL))) {
        if (plugin_chipmunk_env_masks(env->m_env, &monitor->m_collision_mask, str_value) != 0) {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_monitor_load: load mask from '%s' fail!", str_value);
            return -1;
        }
    }

    if ((str_value = cfg_get_string(cfg, "category", NULL))) {
        if (plugin_chipmunk_env_masks(env->m_env, &monitor->m_collision_category, str_value) != 0) {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_monitor_load: load category from '%s' fail!", str_value);
            return -1;
        }
    }
    
    if ((v = cfg_get_string(cfg, "bodies", NULL))) {
        if (monitor->m_bodies) {
            mem_free(module->m_alloc, monitor->m_bodies);
        }
        monitor->m_bodies = cpe_str_mem_dup(module->m_alloc, v);
        if (monitor->m_bodies == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_monitor_load: alloc fail!");
            return -1;
        }
    }
    
    return 0;
}
