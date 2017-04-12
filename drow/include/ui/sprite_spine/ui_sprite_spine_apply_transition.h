#ifndef UI_SPRITE_SPINE_APPLY_TRANSITION_H
#define UI_SPRITE_SPINE_APPLY_TRANSITION_H
#include "ui_sprite_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_APPLY_TRANSITION_NAME;

ui_sprite_spine_apply_transition_t ui_sprite_spine_apply_transition_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_spine_apply_transition_free(ui_sprite_spine_apply_transition_t apply_transition);

int ui_sprite_spine_apply_transition_set_part(ui_sprite_spine_apply_transition_t apply_transition, const char * part);
int ui_sprite_spine_apply_transition_set_transition(ui_sprite_spine_apply_transition_t apply_transition, const char * transition);
    
#ifdef __cplusplus
}
#endif

#endif
