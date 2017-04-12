#ifndef UI_RUNTIME_SOUND_BACKEND_H
#define UI_RUNTIME_SOUND_BACKEND_H
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ui_runtime_sound_backend_res_install_t)(void * ctx, ui_runtime_sound_res_t res, ui_cache_sound_buf_t buffer);
typedef void (*ui_runtime_sound_backend_res_uninstall_t)(void * ctx, ui_runtime_sound_res_t res);
typedef int (*ui_runtime_sound_backend_chanel_init_t)(void * ctx, ui_runtime_sound_chanel_t chanel);
typedef void (*ui_runtime_sound_backend_chanel_fini_t)(void * ctx, ui_runtime_sound_chanel_t chanel);
typedef int (*ui_runtime_sound_backend_chanel_pause_t)(void * ctx, ui_runtime_sound_chanel_t chanel);
typedef int (*ui_runtime_sound_backend_chanel_resume_t)(void * ctx, ui_runtime_sound_chanel_t chanel);
typedef int (*ui_runtime_sound_backend_chanel_play_t)(void * ctx, ui_runtime_sound_chanel_t chanel, ui_runtime_sound_res_t res, float volumn, uint8_t loop);
typedef void (*ui_runtime_sound_backend_chanel_stop_t)(void * ctx, ui_runtime_sound_chanel_t chanel);
typedef int (*ui_runtime_sound_backend_chanel_set_volumn_t)(void * ctx, ui_runtime_sound_chanel_t chanel, float volumn);
typedef ui_runtime_sound_chanel_state_t (*ui_runtime_sound_backend_chanel_get_state_t)(void * ctx, ui_runtime_sound_chanel_t chanel);

ui_runtime_sound_backend_t
ui_runtime_sound_backend_create(
    ui_runtime_module_t module, const char * name,
    void * ctx,
    ui_runtime_sound_backend_res_install_t res_install,
    ui_runtime_sound_backend_res_uninstall_t res_uninstall,
    uint16_t chanel_capacity,
    ui_runtime_sound_backend_chanel_init_t chanel_init,
    ui_runtime_sound_backend_chanel_fini_t chanel_fini,
    ui_runtime_sound_backend_chanel_play_t chanel_play,
    ui_runtime_sound_backend_chanel_stop_t chanel_stop,
    ui_runtime_sound_backend_chanel_pause_t chanel_pause,
    ui_runtime_sound_backend_chanel_resume_t chanel_resume,
    ui_runtime_sound_backend_chanel_get_state_t chanel_get_state,
    ui_runtime_sound_backend_chanel_set_volumn_t chanel_set_volumn);
    
void ui_runtime_sound_backend_free(ui_runtime_sound_backend_t backend);

ui_runtime_sound_backend_t
ui_runtime_sound_backend_find_by_name(ui_runtime_module_t module, const char * name);

#ifdef __cplusplus
}
#endif

#endif 
