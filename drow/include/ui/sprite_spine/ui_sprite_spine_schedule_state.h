#ifndef UI_SPRITE_SPINE_SCHEDULE_STATE_H
#define UI_SPRITE_SPINE_SCHEDULE_STATE_H
#include "ui_sprite_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_SCHEDULE_STATE_NAME;

ui_sprite_spine_schedule_state_t ui_sprite_spine_schedule_state_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_spine_schedule_state_free(ui_sprite_spine_schedule_state_t send_evt);

int ui_sprite_spine_schedule_state_set_part(ui_sprite_spine_schedule_state_t schedule_state, const char * part);
int ui_sprite_spine_schedule_state_set_loop_count(ui_sprite_spine_schedule_state_t schedule_state, const char * loop_count);

int ui_sprite_spine_schedule_state_add_node(
    ui_sprite_spine_schedule_state_t schedule_state, const char * state, const char * loop_count);
    
#ifdef __cplusplus
}
#endif

#endif
