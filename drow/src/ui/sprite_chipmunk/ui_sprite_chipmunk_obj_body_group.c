#include "ui_sprite_chipmunk_obj_body_group_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_sprite_chipmunk_obj_body_group_init(ui_sprite_chipmunk_obj_body_group_t group) {
    TAILQ_INIT(&group->m_bodies);
}

void ui_sprite_chipmunk_obj_body_group_clear(ui_sprite_chipmunk_obj_body_group_t group) {
    while(!TAILQ_EMPTY(&group->m_bodies)) {
        ui_sprite_chipmunk_obj_body_group_binding_free(TAILQ_FIRST(&group->m_bodies));
    }
}

void ui_sprite_chipmunk_obj_body_group_fini(ui_sprite_chipmunk_obj_body_group_t group) {
    while(!TAILQ_EMPTY(&group->m_bodies)) {
        ui_sprite_chipmunk_obj_body_group_binding_free(TAILQ_FIRST(&group->m_bodies));
    }
}
    
uint8_t ui_sprite_chipmunk_obj_body_group_have_body(ui_sprite_chipmunk_obj_body_group_t group, ui_sprite_chipmunk_obj_body_t body) {
    ui_sprite_chipmunk_obj_body_group_binding_t binding;

    TAILQ_FOREACH(binding, &body->m_body_groups, m_next_for_body) {
        if (binding->m_group == group) return 1;
    }

    return 0;
}

ui_sprite_chipmunk_obj_body_group_binding_t
ui_sprite_chipmunk_obj_body_group_binding_create(
    ui_sprite_chipmunk_obj_body_group_t group,
    ui_sprite_chipmunk_obj_body_t body)
{
    ui_sprite_chipmunk_module_t module = body->m_obj->m_env->m_module;
    ui_sprite_chipmunk_obj_body_group_binding_t binding;

    binding = TAILQ_FIRST(&module->m_free_body_group_bindings);
    if (binding == NULL) {
        binding = (ui_sprite_chipmunk_obj_body_group_binding_t)mem_alloc(module->m_alloc, sizeof(struct ui_sprite_chipmunk_obj_body_group_binding));
        if (binding == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_body_group_binding_create: alloc fail!");
            return NULL;
        }
    }
    else {
        TAILQ_REMOVE(&module->m_free_body_group_bindings, binding, m_next_for_group);
    }

    binding->m_group = group;
    binding->m_body = body;

    TAILQ_INSERT_TAIL(&group->m_bodies, binding, m_next_for_group);
    TAILQ_INSERT_TAIL(&body->m_body_groups, binding, m_next_for_body);

    return binding;
}
    
void ui_sprite_chipmunk_obj_body_group_binding_free(ui_sprite_chipmunk_obj_body_group_binding_t binding) {
    ui_sprite_chipmunk_module_t module = binding->m_body->m_obj->m_env->m_module;
    
    TAILQ_REMOVE(&binding->m_group->m_bodies, binding, m_next_for_group);
    TAILQ_REMOVE(&binding->m_body->m_body_groups, binding, m_next_for_body);

    binding->m_group = (ui_sprite_chipmunk_obj_body_group_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_body_group_bindings, binding, m_next_for_group);
}
    
void ui_sprite_chipmunk_obj_body_group_binding_real_free(ui_sprite_chipmunk_obj_body_group_binding_t binding) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)binding->m_group;

    TAILQ_REMOVE(&module->m_free_body_group_bindings, binding, m_next_for_group);
    mem_free(module->m_alloc, binding);
}
    
#ifdef __cplusplus
}
#endif
