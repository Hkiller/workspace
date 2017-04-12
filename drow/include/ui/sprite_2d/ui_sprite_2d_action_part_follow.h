#ifndef UI_SPRITE_2D_ACTION_PART_FOLLOW_H
#define UI_SPRITE_2D_ACTION_PART_FOLLOW_H
#include "ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_2D_ACTION_PART_FOLLOW_NAME;

ui_sprite_2d_action_part_follow_t ui_sprite_2d_action_part_follow_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_2d_action_part_follow_free(ui_sprite_2d_action_part_follow_t action_part_follow);

#ifdef __cplusplus
}
#endif

#endif
