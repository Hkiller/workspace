#ifndef UI_RUNTIME_SOUND_CHANEL_I_H
#define UI_RUNTIME_SOUND_CHANEL_I_H
#include "render/runtime/ui_runtime_sound_chanel.h"
#include "ui_runtime_sound_group_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_sound_chanel {
    ui_runtime_sound_group_t m_group;
    TAILQ_ENTRY(ui_runtime_sound_chanel) m_next_for_group;
    ui_runtime_sound_backend_t m_backend;
    TAILQ_ENTRY(ui_runtime_sound_chanel) m_next_for_backend;
    uint16_t m_id;
    uint8_t m_pause;
    ui_runtime_sound_playing_list_t m_playings;
};

void ui_runtime_sound_chanel_sync_pause(ui_runtime_sound_chanel_t chanel);

/* ALint ui_runtime_sound_chanel_state'(ui_runtime_sound_chanel_t chanel); */
/* const char * ui_runtime_sound_chanel_state_str(ui_runtime_sound_chanel_t chanel); */

#ifdef __cplusplus
}
#endif

#endif 
