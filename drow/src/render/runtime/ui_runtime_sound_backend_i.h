#ifndef UI_RUNTIME_SOUND_BACKEND_I_H
#define UI_RUNTIME_SOUND_BACKEND_I_H
#include "render/runtime/ui_runtime_sound_backend.h"
#include "ui_runtime_sound_group_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_sound_backend {
    ui_runtime_module_t m_module;
    TAILQ_ENTRY(ui_runtime_sound_backend) m_next;
    ui_runtime_sound_chanel_list_t m_chanels;
    char m_name[64];
    void * m_ctx;
    ui_runtime_sound_backend_res_install_t m_res_install;
    ui_runtime_sound_backend_res_uninstall_t m_res_uninstall;
    uint16_t m_chanel_capacity;
    ui_runtime_sound_backend_chanel_init_t m_chanel_init;
    ui_runtime_sound_backend_chanel_fini_t m_chanel_fini;
    ui_runtime_sound_backend_chanel_pause_t m_chanel_pause;
    ui_runtime_sound_backend_chanel_resume_t m_chanel_resume;
    ui_runtime_sound_backend_chanel_play_t m_chanel_play;
    ui_runtime_sound_backend_chanel_stop_t m_chanel_stop;
    ui_runtime_sound_backend_chanel_set_volumn_t m_chanel_set_volumn;
    ui_runtime_sound_backend_chanel_get_state_t m_chanel_get_state;
};

ui_runtime_sound_backend_t ui_runtime_sound_backend_find(ui_runtime_module_t module, ui_runtime_sound_type_t sound_type);
    
#ifdef __cplusplus
}
#endif

#endif 
