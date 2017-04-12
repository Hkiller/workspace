#ifndef UI_SPRITE_CTRL_TRACK_FOLLOW_H
#define UI_SPRITE_CTRL_TRACK_FOLLOW_H
#include "ui_sprite_ctrl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CTRL_TRACK_FOLLOW_NAME;

ui_sprite_ctrl_track_follow_t ui_sprite_ctrl_track_follow_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ctrl_track_follow_free(ui_sprite_ctrl_track_follow_t track_follow);

#ifdef __cplusplus
}
#endif

#endif
