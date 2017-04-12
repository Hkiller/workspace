#ifndef UI_SPRITE_WORLD_H
#define UI_SPRITE_WORLD_H
#include "cpe/tl/tl_types.h"
#include "cpe/xcalc/xcalc_types.h"
#include "cpe/timer/timer_types.h"
#include "ui_sprite_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ui_sprite_world_update_fun_t)(ui_sprite_world_t world, void * ctx, float delta_s);

ui_sprite_world_t ui_sprite_world_create(ui_sprite_repository_t repo, const char * name);
void ui_sprite_world_free(ui_sprite_world_t world);

ui_sprite_world_t ui_sprite_world_find(ui_sprite_repository_t repo, const char * name);

const char * ui_sprite_world_name(ui_sprite_world_t world);
gd_app_context_t ui_sprite_world_app(ui_sprite_world_t world);
ui_sprite_repository_t ui_sprite_world_repository(ui_sprite_world_t world);

ui_sprite_event_handler_t ui_sprite_world_add_event_handler(
    ui_sprite_world_t world, const char * event_name, ui_sprite_event_process_fun_t fun, void * ctx);

void ui_sprite_world_clear_event_handler_by_ctx(ui_sprite_world_t world, void * ctx);

float ui_sprite_world_tick_adj(ui_sprite_world_t world);
void ui_sprite_world_set_tick_adj(ui_sprite_world_t world, float tick_adj);

xcomputer_t ui_sprite_world_computer(ui_sprite_world_t world);

uint8_t ui_sprite_world_is_tick_start(ui_sprite_world_t world);
int ui_sprite_world_start_tick(ui_sprite_world_t world);
void ui_sprite_world_stop_tick(ui_sprite_world_t world);
void ui_sprite_world_tick(ui_sprite_world_t world, float delta_s);

int ui_sprite_world_add_updator(ui_sprite_world_t world, ui_sprite_world_update_fun_t fun, void * ctx);
void ui_sprite_world_remove_updator(ui_sprite_world_t world, void * ctx);
int ui_sprite_world_set_updator_priority(ui_sprite_world_t world, void * ctx, int8_t priority);

ui_sprite_entity_it_t ui_sprite_world_entities(mem_allocrator_t alloc, ui_sprite_world_t world);

/*data*/    
dr_data_t ui_sprite_world_find_data(ui_sprite_world_t world, const char * name);
int ui_sprite_world_set_data(ui_sprite_world_t world, dr_data_t data);    
dr_data_t ui_sprite_world_create_data(ui_sprite_world_t world, LPDRMETA meta, size_t capacity);
int ui_sprite_world_copy_datas(ui_sprite_world_t world, ui_sprite_world_t from_world);

/*attr*/
dr_data_entry_t ui_sprite_world_find_attr(dr_data_entry_t buff, ui_sprite_world_t world, const char * path);
int ui_sprite_world_set_attr(
    ui_sprite_world_t world, const char * path, const char * value, dr_data_source_t data_source);

/*send event operations*/
void ui_sprite_world_send_event(
    ui_sprite_world_t world,
    LPDRMETA meta, void const * data, size_t size);

void ui_sprite_world_send_event_to(
    ui_sprite_world_t world, const char * targets,
    LPDRMETA meta, void const * data, size_t size);

void ui_sprite_world_build_and_send_event(
    ui_sprite_world_t world, const char * event_def, dr_data_source_t data_source);

#ifdef __cplusplus
}
#endif

#endif
