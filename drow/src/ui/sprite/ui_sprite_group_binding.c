#include "ui_sprite_group_binding_i.h"

ui_sprite_group_binding_t
ui_sprite_group_binding_create_group_entity(
    mem_allocrator_t alloc, ui_sprite_group_t group, ui_sprite_entity_t element)
{
    ui_sprite_group_binding_t binding;

    binding = mem_alloc(alloc, sizeof(struct ui_sprite_group_binding));
    if (binding == NULL) return NULL;

    binding->m_group = group;
    binding->m_type = ui_sprite_group_binding_type_entity;
    binding->m_element.m_entity = element;

    TAILQ_INSERT_TAIL(&group->m_elements, binding, m_next_for_group);
    TAILQ_INSERT_TAIL(&element->m_join_groups, binding, m_next_for_element);

    return binding;
}

ui_sprite_group_binding_t
ui_sprite_group_binding_create_group_group(
    mem_allocrator_t alloc, ui_sprite_group_t group, ui_sprite_group_t element)
{
    ui_sprite_group_binding_t binding;

    binding = mem_alloc(alloc, sizeof(struct ui_sprite_group_binding));
    if (binding == NULL) return NULL;

    binding->m_group = group;
    binding->m_type = ui_sprite_group_binding_type_group;
    binding->m_element.m_group = element;

    TAILQ_INSERT_TAIL(&group->m_elements, binding, m_next_for_group);
    TAILQ_INSERT_TAIL(&element->m_join_groups, binding, m_next_for_element);

    return binding;
}

void ui_sprite_group_binding_free(mem_allocrator_t alloc, ui_sprite_group_binding_t binding) {
    TAILQ_REMOVE(&binding->m_group->m_elements, binding, m_next_for_group);
    binding->m_group = NULL;

    if (binding->m_type == ui_sprite_group_binding_type_group) {
        TAILQ_REMOVE(&binding->m_element.m_group->m_join_groups, binding, m_next_for_element);
        binding->m_element.m_group = NULL;
    }
    else {
        TAILQ_REMOVE(&binding->m_element.m_entity->m_join_groups, binding, m_next_for_element);
        binding->m_element.m_entity = NULL;
    }

    mem_free(alloc, binding);
}
