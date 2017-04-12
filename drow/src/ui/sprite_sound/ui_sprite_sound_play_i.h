#ifndef UI_SPRITE_SOUND_PLAY_I_H
#define UI_SPRITE_SOUND_PLAY_I_H
#include "ui/sprite_sound/ui_sprite_sound_play.h"
#include "ui_sprite_sound_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_sound_play {
    ui_sprite_sound_module_t m_module;
    char * m_res;
    uint16_t m_loop_count;
};

int ui_sprite_sound_play_regist(ui_sprite_sound_module_t module);
void ui_sprite_sound_play_unregist(ui_sprite_sound_module_t module);

#ifdef __cplusplus
}
#endif

#endif
