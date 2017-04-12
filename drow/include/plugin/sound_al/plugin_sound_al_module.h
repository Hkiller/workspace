#ifndef DROW_PLUGIN_SOUND_AL_MODULE_H
#define DROW_PLUGIN_SOUND_AL_MODULE_H
#include "gd/app/app_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "plugin_sound_al_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_sound_al_module_t
plugin_sound_al_module_create(
    gd_app_context_t app, ui_cache_manager_t cache_mgr, ui_runtime_module_t runtime,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);
    
void plugin_sound_al_module_free(plugin_sound_al_module_t module);

plugin_sound_al_module_t plugin_sound_al_module_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_sound_al_module_t plugin_sound_al_module_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t plugin_sound_al_module_app(plugin_sound_al_module_t module);
const char * plugin_sound_al_module_name(plugin_sound_al_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif 
