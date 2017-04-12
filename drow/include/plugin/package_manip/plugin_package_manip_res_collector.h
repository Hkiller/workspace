#ifndef DROW_PLUGIN_PACKAGE_MANIP_RES_COLLECTOR_H
#define DROW_PLUGIN_PACKAGE_MANIP_RES_COLLECTOR_H
#include "plugin_package_manip_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*plugin_package_manip_res_collector_fun_t)(void * ctx, plugin_package_package_t package, cfg_t res_cfg, cfg_t package_cfg);

plugin_package_manip_res_collector_t
plugin_package_manip_res_collector_create(
    plugin_package_manip_t manip, const char * name, plugin_package_manip_res_collector_fun_t fun, void * ctx);

void plugin_package_manip_res_collector_free(plugin_package_manip_res_collector_t collector);

plugin_package_manip_res_collector_t
plugin_package_manip_res_collector_find(plugin_package_manip_t manip, const char * name);

#ifdef __cplusplus
}
#endif

#endif
