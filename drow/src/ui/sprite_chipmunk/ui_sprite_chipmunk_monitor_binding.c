#include <assert.h>
#include "ui_sprite_chipmunk_monitor_binding_i.h"
#include "ui_sprite_chipmunk_monitor_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"
#include "ui_sprite_chipmunk_obj_i.h"

ui_sprite_chipmunk_monitor_binding_t
ui_sprite_chipmunk_monitor_binding_create(ui_sprite_chipmunk_obj_body_t body, ui_sprite_chipmunk_monitor_t monitor) {
    ui_sprite_chipmunk_module_t module = body->m_obj->m_env->m_module;
    ui_sprite_chipmunk_monitor_binding_t binding;

    binding = (ui_sprite_chipmunk_monitor_binding_t)mem_alloc(module->m_alloc, sizeof(struct ui_sprite_chipmunk_monitor_binding));
    if (binding == NULL) return NULL;

    binding->m_body = body;
    binding->m_monitor = monitor;
    TAILQ_INSERT_TAIL(&body->m_monitor_bindings, binding, m_next_for_body);
    TAILQ_INSERT_TAIL(&monitor->m_bindings, binding, m_next_for_monitor);

    return binding;
}

void ui_sprite_chipmunk_monitor_binding_free(ui_sprite_chipmunk_monitor_binding_t binding) {
    ui_sprite_chipmunk_module_t module = binding->m_body->m_obj->m_env->m_module;

    TAILQ_REMOVE(&binding->m_body->m_monitor_bindings, binding, m_next_for_body);
    TAILQ_REMOVE(&binding->m_monitor->m_bindings, binding, m_next_for_monitor);
    
    mem_free(module->m_alloc, binding);
}
