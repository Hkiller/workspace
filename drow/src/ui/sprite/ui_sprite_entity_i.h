#ifndef UI_SPRITE_ENTITY_I_H
#define UI_SPRITE_ENTITY_I_H
#include "ui/sprite/ui_sprite_entity.h"
#include "ui_sprite_world_i.h"
#include "ui_sprite_event_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_entity {
    uint32_t m_id;
    const char * m_name;
    uint8_t m_is_proto;
    uint8_t m_is_active;
    uint8_t m_is_wait_destory;
    uint8_t m_debug;
    int8_t m_update_priority;
    ui_sprite_world_t m_world;
    ui_sprite_group_binding_list_t m_join_groups;
    ui_sprite_component_list_t m_components;
    ui_sprite_attr_monitor_list_t m_attr_monitors;
    ui_sprite_event_handler_list_t m_event_handlers;
    struct cpe_hash_entry m_hh_for_id;
    struct cpe_hash_entry m_hh_for_name;
    TAILQ_ENTRY(ui_sprite_entity) m_next_for_wait_destory;
    char m_name_buf[32];
};

uint32_t ui_sprite_entity_id_hash(const ui_sprite_entity_t entity);
int ui_sprite_entity_id_eq(const ui_sprite_entity_t l, const ui_sprite_entity_t r);

uint32_t ui_sprite_entity_name_hash(const ui_sprite_entity_t entity);
int ui_sprite_entity_name_eq(const ui_sprite_entity_t l, const ui_sprite_entity_t r);

void ui_sprite_entity_clear_destoried(ui_sprite_world_t world);

#ifdef __cplusplus
}
#endif

#endif
