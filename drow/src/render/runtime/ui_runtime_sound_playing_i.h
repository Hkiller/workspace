#ifndef UI_RUNTIME_SOUND_PLAYING_I_H
#define UI_RUNTIME_SOUND_PLAYING_I_H
#include "render/runtime/ui_runtime_sound_playing.h"
#include "ui_runtime_sound_chanel_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_sound_playing {
    ui_runtime_sound_chanel_t m_chanel;
    TAILQ_ENTRY(ui_runtime_sound_playing) m_next_for_chanel;
    TAILQ_ENTRY(ui_runtime_sound_playing) m_next_for_module;    
    uint32_t m_id;
    uint8_t m_loop;
    float m_volume;
    ui_cache_res_t m_sound_res;
    float m_stop_duration;
    float m_stop_worked;
};

void ui_runtime_sound_playing_real_free(ui_runtime_sound_playing_t playing);
void ui_runtime_sound_playing_do_set_volume(ui_runtime_sound_playing_t playing);

#ifdef __cplusplus
}
#endif

#endif 
