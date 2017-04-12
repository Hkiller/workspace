#ifndef UI_SPRITE_SPINE_SET_STATE_H
#define UI_SPRITE_SPINE_SET_STATE_H
#include "ui_sprite_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_SET_STATE_NAME;

ui_sprite_spine_set_state_t ui_sprite_spine_set_state_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_spine_set_state_free(ui_sprite_spine_set_state_t set_state);

int ui_sprite_spine_set_state_set_part(ui_sprite_spine_set_state_t set_state, const char * part);
int ui_sprite_spine_set_state_set_state(ui_sprite_spine_set_state_t set_state, const char * state);
    
#ifdef __cplusplus
}
#endif

#endif
