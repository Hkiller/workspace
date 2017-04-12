#ifndef UI_PLUGIN_PARTICLE_MODULE_H
#define UI_PLUGIN_PARTICLE_MODULE_H
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "plugin_particle_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_particle_module_t plugin_particle_module_create(
    gd_app_context_t app,
    ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr, ui_runtime_module_t runtime,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);
    
void plugin_particle_module_free(plugin_particle_module_t cache_mgr);

gd_app_context_t plugin_particle_module_app(plugin_particle_module_t cache_mgr);
const char * plugin_particle_module_name(plugin_particle_module_t cache_mgr);

plugin_particle_module_t plugin_particle_module_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_particle_module_t plugin_particle_module_find_nc(gd_app_context_t app, const char * name);

int plugin_particle_module_collect(plugin_particle_module_t cache_mgr, ui_data_src_t root);

LPDRMETA plugin_particle_module_meta_emitter(plugin_particle_module_t module);
LPDRMETA plugin_particle_module_meta_mod(plugin_particle_module_t module);
LPDRMETA plugin_particle_module_meta_mod_data(plugin_particle_module_t module);        
LPDRMETA plugin_particle_module_meta_point(plugin_particle_module_t module);

#ifdef __cplusplus
}
#endif

#endif

