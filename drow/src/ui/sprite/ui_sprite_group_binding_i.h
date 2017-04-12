#ifndef UI_SPRITE_GROUP_BINDING_I_H
#define UI_SPRITE_GROUP_BINDING_I_H
#include "ui/sprite/ui_sprite_group.h"
#include "ui_sprite_group_i.h"
#include "ui_sprite_entity_i.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ui_sprite_group_binding_type {
    ui_sprite_group_binding_type_entity,
    ui_sprite_group_binding_type_group
};

struct ui_sprite_group_binding {
    ui_sprite_group_t m_group;
    enum ui_sprite_group_binding_type m_type;
    union {
        ui_sprite_entity_t m_entity;
        ui_sprite_group_t m_group;
    } m_element;

    TAILQ_ENTRY(ui_sprite_group_binding) m_next_for_group;
    TAILQ_ENTRY(ui_sprite_group_binding) m_next_for_element;
};

ui_sprite_group_binding_t
ui_sprite_group_binding_create_group_entity(
    mem_allocrator_t alloc, ui_sprite_group_t group, ui_sprite_entity_t element);

ui_sprite_group_binding_t
ui_sprite_group_binding_create_group_group(
    mem_allocrator_t alloc, ui_sprite_group_t group, ui_sprite_group_t element);

void ui_sprite_group_binding_free(
    mem_allocrator_t alloc, ui_sprite_group_binding_t group_binding);

#ifdef __cplusplus
}
#endif

#endif
