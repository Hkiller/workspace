#ifndef UI_SPRITE_CHIPMUNK_APPLY_VELOCITY_H
#define UI_SPRITE_CHIPMUNK_APPLY_VELOCITY_H
#include "ui_sprite_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CHIPMUNK_APPLY_VELOCITY_NAME;

ui_sprite_chipmunk_apply_velocity_t ui_sprite_chipmunk_apply_velocity_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_chipmunk_apply_velocity_free(ui_sprite_chipmunk_apply_velocity_t send_evt);

#ifdef __cplusplus
}
#endif

#endif
