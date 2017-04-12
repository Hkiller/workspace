#ifndef UI_SPRITE_UI_ACTION_PLAY_ANIM_H
#define UI_SPRITE_UI_ACTION_PLAY_ANIM_H
#include "ui_sprite_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_UI_ACTION_PLAY_ANIM_NAME;

ui_sprite_ui_action_play_anim_t ui_sprite_ui_action_play_anim_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ui_action_play_anim_free(ui_sprite_ui_action_play_anim_t play_anim);

int ui_sprite_ui_action_play_anim_set_back_res(ui_sprite_ui_action_play_anim_t play_anim, const char * back_res);

#ifdef __cplusplus
}
#endif

#endif
