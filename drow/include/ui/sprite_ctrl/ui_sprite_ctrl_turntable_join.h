#ifndef UI_SPRITE_CTRL_TURNTABLE_JOIN_H
#define UI_SPRITE_CTRL_TURNTABLE_JOIN_H
#include "gd/app/app_types.h"
#include "ui_sprite_ctrl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CTRL_TURNTABLE_JOIN_NAME;

ui_sprite_ctrl_turntable_join_t ui_sprite_ctrl_turntable_join_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ctrl_turntable_join_free(ui_sprite_ctrl_turntable_join_t ctrl);

const char * ui_sprite_ctrl_turntable_join_turntable(ui_sprite_ctrl_turntable_join_t ctrl);
void ui_sprite_ctrl_turntable_join_set_turntable(ui_sprite_ctrl_turntable_join_t ctrl, const char * turntable);

#ifdef __cplusplus
}
#endif

#endif
