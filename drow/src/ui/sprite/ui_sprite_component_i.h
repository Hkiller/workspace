#ifndef UI_SPRITE_COMPONENT_I_H
#define UI_SPRITE_COMPONENT_I_H
#include "ui/sprite/ui_sprite_component.h"
#include "ui_sprite_entity_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_component {
    ui_sprite_entity_t m_entity;
    ui_sprite_component_meta_t m_meta;
    uint8_t m_is_active;
    uint8_t m_is_update;
    uint8_t m_need_process;
    ui_sprite_event_handler_list_t m_event_handlers;
    ui_sprite_attr_monitor_list_t m_attr_monitors;
    TAILQ_ENTRY(ui_sprite_component) m_next_for_entity;
    TAILQ_ENTRY(ui_sprite_component) m_next_for_meta;
    TAILQ_ENTRY(ui_sprite_component) m_next_for_updating;
};

#ifdef __cplusplus
}
#endif

#endif
