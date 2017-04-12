#ifndef UI_PLUGIN_PACKAGE_MODULE_H
#define UI_PLUGIN_PACKAGE_MODULE_H
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "plugin_package_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_package_module_t plugin_package_module_create(
    gd_app_context_t app,
    ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);
    
void plugin_package_module_free(plugin_package_module_t cache_mgr);

gd_app_context_t plugin_package_module_app(plugin_package_module_t cache_mgr);
const char * plugin_package_module_name(plugin_package_module_t cache_mgr);

plugin_package_module_t plugin_package_module_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_package_module_t plugin_package_module_find_nc(gd_app_context_t app, const char * name);

ui_data_mgr_t plugin_package_module_data_mgr(plugin_package_module_t module);
ui_cache_manager_t plugin_package_module_cache_mgr(plugin_package_module_t module);

void plugin_package_module_packages(plugin_package_module_t module, plugin_package_package_it_t package_it);
void plugin_package_module_regions(plugin_package_module_t module, plugin_package_region_it_t region_it);
void plugin_package_module_loaded_packages(plugin_package_module_t module, plugin_package_package_it_t package_it);
void plugin_package_module_downloading_packages(plugin_package_module_t module, plugin_package_package_it_t package_it);
void plugin_package_module_loading_packages(plugin_package_module_t module, plugin_package_package_it_t package_it);
    
const char * plugin_package_module_repo_path(plugin_package_module_t module);
int plugin_package_module_set_repo_path(plugin_package_module_t module, const char * path);

void plugin_package_module_reinstall(plugin_package_module_t module);
    
int plugin_package_module_install_summary(plugin_package_module_t module);
int plugin_package_module_install_packages(plugin_package_module_t module);

uint8_t plugin_package_module_is_control_res(plugin_package_module_t module);
void plugin_package_module_set_control_res(plugin_package_module_t module, uint8_t control_res);

/*配置 */
uint16_t plugin_package_module_downloading_package_limit(plugin_package_module_t module);
void plugin_package_module_set_downloading_package_limit(plugin_package_module_t module, uint16_t limit);

uint16_t plugin_package_module_loading_package_limit(plugin_package_module_t module);
void plugin_package_module_set_loading_package_limit(plugin_package_module_t module, uint16_t limit);
    
uint16_t plugin_package_module_process_tick_limit_ms(plugin_package_module_t module);
void plugin_package_module_set_process_tick_limit_ms(plugin_package_module_t module, uint16_t limit);
    
/*统计 */
void plugin_package_module_total_reset(plugin_package_module_t module);
uint16_t plugin_package_module_total_download_complete_count(plugin_package_module_t module);
uint16_t plugin_package_module_total_download_count(plugin_package_module_t module);
    
uint16_t plugin_package_module_total_load_complete_count(plugin_package_module_t module);
uint16_t plugin_package_module_total_load_count(plugin_package_module_t module);    

/*回收 */    
void plugin_package_module_gc(plugin_package_module_t module);

#ifdef __cplusplus
}
#endif

#endif

