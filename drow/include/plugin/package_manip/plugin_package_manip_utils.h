#ifndef DROW_PLUGIN_PACKAGE_MANIP_UTILS_H
#define DROW_PLUGIN_PACKAGE_MANIP_UTILS_H
#include "plugin_package_manip_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int plugin_package_manip_collect_src_by_res(ui_data_src_group_t group, const char * res);

typedef int (*plugin_package_manip_collect_op_fun_t)(
    void * ctx, plugin_package_package_t package, cfg_t res_cfg, cfg_t package_cfg, cfg_calc_context_t args);
    
int plugin_package_manip_collect_search(
    plugin_package_manip_t manip, plugin_package_package_t package, cfg_t res_cfg, cfg_t package_cfg, cfg_calc_context_t args,
    const char * path, plugin_package_manip_collect_op_fun_t op, void * ctx);

/*将扩展包中所有重复的资源收集起来(不递归) */
int plugin_package_manip_collect_extern_shared(plugin_package_package_t base_package);

/*将基础包中已经提供的资源移除 */    
int plugin_package_manip_remove_base_provided(plugin_package_package_t package);
    
#ifdef __cplusplus
}
#endif

#endif
