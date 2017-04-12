#ifndef UI_SPRITE_CHIPMUNK_WITH_ADDITION_ACCEL_H
#define UI_SPRITE_CHIPMUNK_WITH_ADDITION_ACCEL_H
#include "ui_sprite_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CHIPMUNK_WITH_ADDITION_ACCEL_NAME;

ui_sprite_chipmunk_with_addition_accel_t ui_sprite_chipmunk_with_addition_accel_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_chipmunk_with_addition_accel_free(ui_sprite_chipmunk_with_addition_accel_t send_evt);

#ifdef __cplusplus
}
#endif

#endif
