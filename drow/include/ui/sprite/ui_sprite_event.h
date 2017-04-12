#ifndef UI_SPRITE_EVENT_H
#define UI_SPRITE_EVENT_H
#include "ui_sprite_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_sprite_event_handler_free(ui_sprite_world_t world, ui_sprite_event_handler_t handler);

const char * ui_sprite_event_handler_process_event(ui_sprite_event_handler_t handler);
ui_sprite_event_process_fun_t ui_sprite_event_handler_process_fun(ui_sprite_event_handler_t handler);
void * ui_sprite_event_handler_process_ctx(ui_sprite_event_handler_t handler);

ui_sprite_event_t ui_sprite_event_copy(mem_allocrator_t alloc, ui_sprite_event_t evt);

#ifdef __cplusplus
}
#endif

#endif
