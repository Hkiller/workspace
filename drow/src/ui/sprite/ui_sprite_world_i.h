#ifndef UI_SPRITE_WORLD_I_H
#define UI_SPRITE_WORLD_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/sorted_vector.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui_sprite_repository_i.h"
#include "ui_sprite_event_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_world_updator * ui_sprite_world_updator_t;
typedef struct ui_sprite_world_data * ui_sprite_world_data_t;

typedef TAILQ_HEAD(ui_sprite_update_node_list, ui_sprite_update_node) ui_sprite_update_node_list_t;
typedef TAILQ_HEAD(ui_sprite_world_res_list, ui_sprite_world_res) ui_sprite_world_res_list_t;
typedef TAILQ_HEAD(ui_sprite_world_data_list, ui_sprite_world_data) ui_sprite_world_data_list_t;    
typedef struct ui_sprite_update_node * ui_sprite_update_node_t;

struct ui_sprite_update_node {
    ui_sprite_component_list_t m_components;
    TAILQ_ENTRY(ui_sprite_update_node) m_next;
};

struct ui_sprite_world_updator {
    int8_t m_update_priority;
    ui_sprite_world_update_fun_t m_fun;
    void * m_ctx;
};

struct ui_sprite_world {
    ui_sprite_repository_t m_repo;
    char m_name[64];
    TAILQ_ENTRY(ui_sprite_world) m_next_for_repo;

    uint32_t m_max_entity_id;
    uint8_t m_in_tick;
	float m_tick_adj;
    struct cpe_hash_table m_entity_by_id;
    struct cpe_hash_table m_entity_by_name;
    struct cpe_hash_table m_entity_protos;
    struct cpe_hash_table m_group_by_id;
    struct cpe_hash_table m_group_by_name;
    struct cpe_hash_table m_resources;
    ui_sprite_world_res_list_t m_resources_by_order;
    ui_sprite_world_data_list_t m_datas;
    struct cpe_hash_table m_event_handlers;
    struct cpe_hash_table m_attr_monitor_bindings;
    ui_sprite_entity_list_t m_wait_destory_entities;
    ui_sprite_pending_event_list_t m_pending_events;
    ui_sprite_attr_monitor_list_t m_pending_monitors;
    void * m_evt_processed_entities_buf;
    struct cpe_sorted_vector m_evt_processed_entities;
    uint8_t m_updator_count;
    struct ui_sprite_world_updator m_updators[64];
    ui_sprite_update_node_list_t m_updating_components;
    struct ui_sprite_update_node m_updating_nodes[256];
};

void ui_sprite_component_enqueue(ui_sprite_world_t world, ui_sprite_component_t component, int8_t priority);
void ui_sprite_component_dequeue(ui_sprite_world_t world, ui_sprite_component_t component, int8_t priority);

void ui_sprite_world_update_components(ui_sprite_world_t world, void * ctx, float delta_s);

#ifdef __cplusplus
}
#endif

#endif
