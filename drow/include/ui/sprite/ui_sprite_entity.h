#ifndef UI_SPRITE_ENTITYR_H
#define UI_SPRITE_ENTITYR_H
#include "ui_sprite_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*entity proto*/
ui_sprite_entity_t ui_sprite_entity_create(ui_sprite_world_t world, const char * name, const char * proto_name);
void ui_sprite_entity_free(ui_sprite_entity_t entity);
void ui_sprite_entity_free_all(ui_sprite_world_t world);

ui_sprite_entity_t ui_sprite_entity_find_by_id(ui_sprite_world_t world, uint32_t id);
ui_sprite_entity_t ui_sprite_entity_find_by_name(ui_sprite_world_t world, const char * name);
ui_sprite_entity_t ui_sprite_entity_find_auto_select(ui_sprite_world_t world, const char * def);

void ui_sprite_entity_set_destory(ui_sprite_entity_t entity);

uint32_t ui_sprite_entity_id(ui_sprite_entity_t entity);
const char * ui_sprite_entity_name(ui_sprite_entity_t entity);
ui_sprite_world_t ui_sprite_entity_world(ui_sprite_entity_t entity);

uint8_t ui_sprite_entity_debug(ui_sprite_entity_t entity);
void ui_sprite_entity_set_debug(ui_sprite_entity_t entity, uint8_t is_debug);

int8_t ui_sprite_entity_update_priority(ui_sprite_entity_t entity);
void ui_sprite_entity_set_update_priority(ui_sprite_entity_t entity, int8_t priority);

int ui_sprite_entity_enter(ui_sprite_entity_t entity);
void ui_sprite_entity_exit(ui_sprite_entity_t entity);

uint8_t ui_sprite_entity_is_active(ui_sprite_entity_t entity);

ui_sprite_event_handler_t ui_sprite_entity_add_event_handler(
    ui_sprite_entity_t entity, ui_sprite_event_scope_t scope,
    const char * event_name, ui_sprite_event_process_fun_t fun, void * ctx);

void ui_sprite_entity_send_event(
    ui_sprite_entity_t entity,
    LPDRMETA meta, void const * data, size_t size);

ui_sprite_event_t ui_sprite_entity_build_event(
    ui_sprite_entity_t entity, mem_allocrator_t alloc,
    const char * event_def, dr_data_source_t data_source);

void ui_sprite_entity_build_and_send_event(
    ui_sprite_entity_t entity, const char * event_def, dr_data_source_t data_source);

void ui_sprite_entity_check_build_and_send_event(
    ui_sprite_entity_t entity, const char * event_def, dr_data_source_t data_source);

/*entity proto*/
ui_sprite_entity_t ui_sprite_entity_proto_create(ui_sprite_world_t world, const char * proto_name);
ui_sprite_entity_t ui_sprite_entity_proto_find(ui_sprite_world_t world, const char * proto_name);
void ui_sprite_entity_proto_free_all(ui_sprite_world_t world);

/*entity iterator*/
ui_sprite_entity_t ui_sprite_entity_it_next(ui_sprite_entity_it_t entity_id);
void ui_sprite_entity_it_free(ui_sprite_entity_it_t entity_id);

#ifdef __cplusplus
}
#endif

#endif
