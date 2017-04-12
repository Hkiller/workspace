#ifndef UI_SPRITE_CHIPMUNK_MANIPULATOR_H
#define UI_SPRITE_CHIPMUNK_MANIPULATOR_H
#include "ui_sprite_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CHIPMUNK_MANIPULATOR_NAME;

ui_sprite_chipmunk_manipulator_t ui_sprite_chipmunk_manipulator_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_chipmunk_manipulator_free(ui_sprite_chipmunk_manipulator_t send_evt);

#ifdef __cplusplus
}
#endif

#endif
