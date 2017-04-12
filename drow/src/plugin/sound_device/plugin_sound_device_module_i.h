#ifndef PLUGIN_SOUND_MODULE_DEVICE_I_H
#define PLUGIN_SOUND_MODULE_DEVICE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "gd/app/app_context.h"
#include "render/runtime/ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_sound_device_module * plugin_sound_device_module_t;
typedef struct plugin_sound_device_backend * plugin_sound_device_backend_t;
    
struct plugin_sound_device_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    ui_cache_manager_t m_cache_mgr;
    ui_runtime_module_t m_runtime;
    uint8_t m_debug;
    plugin_sound_device_backend_t m_backend;
};

int plugin_sound_device_module_init_backend(plugin_sound_device_module_t module);
void plugin_sound_device_module_fini_backend(plugin_sound_device_module_t module);

#ifdef __cplusplus
}
#endif

#endif 
