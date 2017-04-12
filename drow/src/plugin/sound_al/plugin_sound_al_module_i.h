#ifndef PLUGIN_SOUND_MODULE_AL_I_H
#define PLUGIN_SOUND_MODULE_AL_I_H
#ifdef _APPLE
#include "openal/al.h"
#include "openal/alc.h"
#else
#include "AL/al.h"
#include "AL/alc.h"
#endif
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "gd/app/app_context.h"
#include "plugin/sound_al/plugin_sound_al_module.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_sound_al_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    ui_cache_manager_t m_cache_mgr;
    ui_runtime_module_t m_runtime;
    uint8_t m_debug;

    ALCdevice * m_device;
    ALCcontext * m_context;
};

int plugin_sound_al_module_init_device(plugin_sound_al_module_t module);
void plugin_sound_al_module_fini_device(plugin_sound_al_module_t module);
    
int plugin_sound_al_module_init_context(plugin_sound_al_module_t module);
void plugin_sound_al_module_fini_context(plugin_sound_al_module_t module);

int plugin_sound_al_module_init_sound_backend(plugin_sound_al_module_t module);
void plugin_sound_al_module_fini_sound_backend(plugin_sound_al_module_t module);
    
int plugin_sound_al_module_init_backend(plugin_sound_al_module_t module);
void plugin_sound_al_module_fini_backend(plugin_sound_al_module_t module);
    
const char * plugin_sound_al_module_error_str(ALenum err);
void plugin_sound_al_module_sync_suspend(plugin_sound_al_module_t module);

#ifdef __cplusplus
}
#endif

#endif 
