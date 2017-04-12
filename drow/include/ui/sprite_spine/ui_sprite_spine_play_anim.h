#ifndef UI_SPRITE_SPINE_PLAY_ANIM_H
#define UI_SPRITE_SPINE_PLAY_ANIM_H
#include "ui_sprite_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_PLAY_ANIM_NAME;

ui_sprite_spine_play_anim_t ui_sprite_spine_play_anim_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_spine_play_anim_free(ui_sprite_spine_play_anim_t send_evt);

const char * ui_sprite_spine_play_anim_def(ui_sprite_spine_play_anim_t play_anim);
int ui_sprite_spine_play_anim_set_def(ui_sprite_spine_play_anim_t play_anim, const char * anim_def);

#ifdef __cplusplus
}
#endif

#endif
