#ifndef UI_SPRITE_CTRL_TURNTABLE_ACTIVE_H
#define UI_SPRITE_CTRL_TURNTABLE_ACTIVE_H
#include "gd/app/app_types.h"
#include "ui_sprite_ctrl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CTRL_TURNTABLE_ACTIVE_NAME;

ui_sprite_ctrl_turntable_active_t ui_sprite_ctrl_turntable_active_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ctrl_turntable_active_free(ui_sprite_ctrl_turntable_active_t ctrl);

int ui_sprite_ctrl_turntable_active_set_decorator(ui_sprite_ctrl_turntable_active_t active, const char * decorator);

#ifdef __cplusplus
}
#endif

#endif
