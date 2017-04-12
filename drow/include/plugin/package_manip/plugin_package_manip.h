#ifndef DROW_PLUGIN_PACKAGE_MANIP_H
#define DROW_PLUGIN_PACKAGE_MANIP_H
#include "plugin_package_manip_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_package_manip_t
plugin_package_manip_create(
    gd_app_context_t app, ui_ed_mgr_t ed_mgr, plugin_package_module_t package_module,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);

void plugin_package_manip_free(plugin_package_manip_t manip);

plugin_package_manip_t plugin_package_manip_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_package_manip_t plugin_package_manip_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t plugin_package_manip_app(plugin_package_manip_t manip);
const char * plugin_package_manip_name(plugin_package_manip_t manip);

xcomputer_t plugin_package_manip_computer(plugin_package_manip_t manip);
const char * plugin_package_manip_calc(plugin_package_manip_t manip, mem_buffer_t buffer, const char * def, cfg_calc_context_t args);
    
int plugin_package_manip_build(plugin_package_manip_t manip, cfg_t cfg);
    
#ifdef __cplusplus
}
#endif

#endif
