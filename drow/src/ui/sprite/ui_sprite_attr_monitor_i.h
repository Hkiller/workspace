#ifndef UI_SPRITE_ATTR_MONITOR_I_H
#define UI_SPRITE_ATTR_MONITOR_I_H
#include "cpe/pal/pal_queue.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui_sprite_repository_i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*ui_sprite_attr_monitor*/
enum ui_sprite_attr_monitor_state {
    ui_sprite_attr_monitor_idle,
    ui_sprite_attr_monitor_working,
    ui_sprite_attr_monitor_deleting
};

struct ui_sprite_attr_monitor {
    enum ui_sprite_attr_monitor_state m_state;
    uint32_t m_entity_id;
    uint8_t m_is_triggered;
    ui_sprite_attr_monitor_fun_t m_fun;
    void * m_ctx;
    ui_sprite_attr_monitor_binding_list_t m_bindings;

    ui_sprite_attr_monitor_list_t * m_manage;
    TAILQ_ENTRY(ui_sprite_attr_monitor) m_next_for_manage;
    TAILQ_ENTRY(ui_sprite_attr_monitor) m_next_for_pending;
};

ui_sprite_attr_monitor_t
ui_sprite_attr_monitor_create(
    ui_sprite_world_t world, ui_sprite_attr_monitor_list_t * manage,
    ui_sprite_entity_t entity,
    ui_sprite_attr_monitor_fun_t fun, void * ctx);

void ui_sprite_attr_monitor_free(
    ui_sprite_world_t world,
    ui_sprite_attr_monitor_t attr_monitor);

void ui_sprite_attr_monitor_clear_all(ui_sprite_world_t world, ui_sprite_attr_monitor_list_t * manage);

int ui_sprite_attr_monitor_bind_attrs(
    ui_sprite_world_t world, ui_sprite_entity_t entity,
    ui_sprite_attr_monitor_t monitor, const char * input_attrs);

int ui_sprite_attr_monitor_bind_by_def(
    ui_sprite_world_t world, ui_sprite_entity_t entity,
    ui_sprite_attr_monitor_t monitor, const char * def);
    
void ui_sprite_attr_monitor_set_triggered(ui_sprite_world_t world, ui_sprite_attr_monitor_t attr_monitor, uint8_t is_triggered);

void ui_sprite_attr_monitor_notify(ui_sprite_world_t world, uint32_t entity_id, const char * attr_name);
void ui_sprite_attr_monitor_process(ui_sprite_world_t world);

/*attr monitor binding*/
struct ui_sprite_attr_monitor_binding {
    const char * m_name;
    ui_sprite_attr_monitor_t m_monitor;
    struct cpe_hash_entry m_hh_for_world;
    TAILQ_ENTRY(ui_sprite_attr_monitor_binding) m_next_for_binding;
};

int ui_sprite_attr_monitor_binding_create_to_last_entry(
    ui_sprite_world_t world, ui_sprite_entity_t entity,
    ui_sprite_attr_monitor_t monitor, const char * name);

ui_sprite_attr_monitor_binding_t
ui_sprite_attr_monitor_binding_create(ui_sprite_world_t world, ui_sprite_attr_monitor_t monitor, const char * name);

void ui_sprite_attr_monitor_binding_free(ui_sprite_world_t world, ui_sprite_attr_monitor_binding_t binding);

uint32_t ui_sprite_attr_monitor_binding_hash(ui_sprite_attr_monitor_binding_t ep);
int ui_sprite_attr_monitor_binding_eq(ui_sprite_attr_monitor_binding_t l, ui_sprite_attr_monitor_binding_t r);

#ifdef __cplusplus
}
#endif

#endif

