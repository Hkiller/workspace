#ifndef UI_SPRITE_CTRL_TRACK_MANIP_H
#define UI_SPRITE_CTRL_TRACK_MANIP_H
#include "ui_sprite_ctrl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CTRL_TRACK_MANIP_NAME;

ui_sprite_ctrl_track_manip_t ui_sprite_ctrl_track_manip_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ctrl_track_manip_free(ui_sprite_ctrl_track_manip_t track_manip);

#ifdef __cplusplus
}
#endif

#endif
