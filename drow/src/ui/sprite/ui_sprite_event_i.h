#ifndef UI_SPRITE_EVENT_I_H
#define UI_SPRITE_EVENT_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "ui/sprite/ui_sprite_event.h"
#include "ui_sprite_repository_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_sprite_event_handler_list, ui_sprite_event_handler) ui_sprite_event_handler_list_t;

typedef struct ui_sprite_pending_event * ui_sprite_pending_event_t;
typedef TAILQ_HEAD(ui_sprite_pending_event_list, ui_sprite_pending_event) ui_sprite_pending_event_list_t;

/*ui_sprite_event_handler*/
enum ui_sprite_event_handler_state {
    ui_sprite_event_handler_idle,
    ui_sprite_event_handler_working,
    ui_sprite_event_handler_deleting
};

struct ui_sprite_event_handler {
    enum ui_sprite_event_handler_state m_state;
    const char * m_event_name;
    uint32_t m_event_entity;
    ui_sprite_event_process_fun_t m_fun;
    void * m_ctx;

    struct cpe_hash_entry m_hh_for_world;
    ui_sprite_event_handler_list_t * m_manage;
    TAILQ_ENTRY(ui_sprite_event_handler) m_next_for_manage;
};

ui_sprite_event_handler_t
ui_sprite_event_handler_create(
    ui_sprite_world_t world, ui_sprite_event_handler_list_t * manage,
    const char * event_name, uint32_t event_entity,
    ui_sprite_event_process_fun_t fun, void * ctx);

void ui_sprite_event_handler_free(
    ui_sprite_world_t world,
    ui_sprite_event_handler_t event_handler);

void ui_sprite_event_handler_clear_all(ui_sprite_world_t world, ui_sprite_event_handler_list_t * manage);

uint32_t ui_sprite_event_handler_hash(ui_sprite_event_handler_t ep);
int ui_sprite_event_handler_eq(ui_sprite_event_handler_t l, ui_sprite_event_handler_t r);

/*ui_sprite_pending_event*/
enum ui_sprite_event_target_type {
    ui_sprite_event_target_type_world,
    ui_sprite_event_target_type_group,
    ui_sprite_event_target_type_entity,
};

struct ui_sprite_event_target {
    enum ui_sprite_event_target_type m_type;
    union {
        uint32_t to_group_id;
        uint32_t to_entity_id;
    } m_data;
};

struct ui_sprite_pending_event {
    uint8_t m_debug_level;
    uint8_t m_target_count;
    struct ui_sprite_event_target m_targets[10];
    struct ui_sprite_event m_data;
    TAILQ_ENTRY(ui_sprite_pending_event) m_next;
};

void ui_sprite_event_dispatch(ui_sprite_world_t world);

ui_sprite_pending_event_t
ui_sprite_event_enqueue(
    ui_sprite_world_t world, ui_sprite_entity_t from_entity,
    uint8_t debug_level, LPDRMETA meta, void const * data, size_t size);

void ui_sprite_pending_event_free(
    ui_sprite_world_t world, ui_sprite_pending_event_t evt);

int ui_sprite_event_add_target(
    ui_sprite_pending_event_t processing_evt,
    ui_sprite_entity_t from_entity,
    ui_sprite_world_t world, const char * target,
    dr_data_source_t data_source);

void ui_sprite_event_build_and_enqueue(
    ui_sprite_world_t world, ui_sprite_entity_t from_entity,
    const char * event_def, dr_data_source_t data_source, uint8_t ignore_not_exist);

int ui_sprite_event_analize_head(
    ui_sprite_event_meta_t * r_event_meta, char ** r_event_args,
    ui_sprite_repository_t repo, char * event_def, uint8_t ignore_not_exist);

int ui_sprite_event_analize_targets(
    ui_sprite_pending_event_t processing_evt,
    ui_sprite_world_t world, ui_sprite_entity_t from_entity, char * targets,
    dr_data_source_t data_source);

#ifdef __cplusplus
}
#endif

#endif

