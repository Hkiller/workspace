#ifndef UI_SPRITE_SOUND_PLAY_H
#define UI_SPRITE_SOUND_PLAY_H
#include "ui_sprite_sound_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SOUND_PLAY_NAME;

ui_sprite_sound_play_t ui_sprite_sound_play_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_sound_play_free(ui_sprite_sound_play_t send_evt);

#ifdef __cplusplus
}
#endif

#endif
