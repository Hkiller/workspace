#ifndef PLUGIN_SOUND_MODULE_DEVICE_BACKEND_I_H
#define PLUGIN_SOUND_MODULE_DEVICE_BACKEND_I_H
#import <AudioToolbox/AudioToolbox.h>
#include "../plugin_sound_device_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_sound_device_backend {
    int a;
};
    
int plugin_sound_device_res_install(void * ctx, ui_runtime_sound_res_t res, ui_cache_sound_buf_t sound_buf);
void plugin_sound_device_res_uninstall(void * ctx, ui_runtime_sound_res_t res);
int plugin_sound_device_chanel_init(void * ctx, ui_runtime_sound_chanel_t chanel);
void plugin_sound_device_chanel_fini(void * ctx, ui_runtime_sound_chanel_t chanel);
int plugin_sound_device_chanel_pause(void * ctx, ui_runtime_sound_chanel_t chanel);
int plugin_sound_device_chanel_resume(void * ctx, ui_runtime_sound_chanel_t chanel);
int plugin_sound_device_chanel_play(void * ctx, ui_runtime_sound_chanel_t chanel, ui_runtime_sound_res_t res, float volumn, uint8_t loop);
void plugin_sound_device_chanel_stop(void * ctx, ui_runtime_sound_chanel_t chanel);
int plugin_sound_device_chanel_set_volumn(void * ctx, ui_runtime_sound_chanel_t chanel, float volumn);
ui_runtime_sound_chanel_state_t plugin_sound_device_chanel_get_state(void * ctx, ui_runtime_sound_chanel_t chanel);

#ifdef __cplusplus
}
#endif
    
#endif

