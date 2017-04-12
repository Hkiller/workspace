#ifndef UI_SPRITE_COMPONENT_H
#define UI_SPRITE_COMPONENT_H
#include "ui_sprite_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_component_t ui_sprite_component_create(ui_sprite_entity_t entity, const char * type);
void ui_sprite_component_free(ui_sprite_component_t component);
ui_sprite_component_t ui_sprite_component_clone(ui_sprite_entity_t entity, ui_sprite_component_t from);

ui_sprite_component_t ui_sprite_component_find(ui_sprite_entity_t entity, const char * type);

const char * ui_sprite_component_name(ui_sprite_component_t component);
ui_sprite_component_meta_t ui_sprite_component_meta(ui_sprite_component_t component);
void * ui_sprite_component_data(ui_sprite_component_t component);
size_t ui_sprite_component_data_size(ui_sprite_component_t component);
ui_sprite_entity_t ui_sprite_component_entity(ui_sprite_component_t component);

ui_sprite_component_t ui_sprite_component_from_data(void * data);

void ui_sprite_component_send_event(
    ui_sprite_component_t component,
    LPDRMETA meta, void const * data, size_t size);

void ui_sprite_component_send_event_to(
    ui_sprite_component_t component, const char * target,
    LPDRMETA meta, void const * data, size_t size);

void ui_sprite_component_build_and_send_event(
    ui_sprite_component_t component,
    const char * event_def, dr_data_source_t data_source);

ui_sprite_event_handler_t
ui_sprite_component_add_event_handler(
    ui_sprite_component_t component, ui_sprite_event_scope_t scope,
    const char * event_name,
    ui_sprite_event_process_fun_t fun, void * ctx);

ui_sprite_event_handler_t
ui_sprite_component_find_event_handler(
    ui_sprite_component_t component, const char * event_name);

void ui_sprite_component_remove_event_handler(
    ui_sprite_component_t component, const char * event_name);

void ui_sprite_component_clear_event_handlers(ui_sprite_component_t component);

uint8_t ui_sprite_component_is_active(ui_sprite_component_t component);

ui_sprite_attr_monitor_t
ui_sprite_component_add_attr_monitor(
    ui_sprite_component_t component, const char * attrs, ui_sprite_attr_monitor_fun_t fun, void * ctx);

ui_sprite_attr_monitor_t
ui_sprite_component_add_attr_monitor_by_def(
    ui_sprite_component_t component, const char * def, ui_sprite_attr_monitor_fun_t fun, void * ctx);

ui_sprite_attr_monitor_t
ui_sprite_component_add_attr_monitor_by_defs(
    ui_sprite_component_t component, const char * * defs, uint16_t def_count,
    ui_sprite_attr_monitor_fun_t fun, void * ctx);
    
void ui_sprite_component_clear_attr_monitors(ui_sprite_component_t component);

int ui_sprite_component_enter(ui_sprite_component_t component);
void ui_sprite_component_exit(ui_sprite_component_t component);

uint8_t ui_sprite_component_is_update(ui_sprite_component_t component);
int ui_sprite_component_start_update(ui_sprite_component_t component);
void ui_sprite_component_stop_update(ui_sprite_component_t component);
void ui_sprite_component_sync_update(ui_sprite_component_t component, uint8_t is_start);

dr_data_t ui_sprite_component_attr_data(dr_data_t buff, ui_sprite_component_t component);
int ui_sprite_component_set_attr_data(ui_sprite_component_t component, dr_data_t);

#ifdef __cplusplus
}
#endif

#endif
