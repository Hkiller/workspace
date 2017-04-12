#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "plugin/package_manip/plugin_package_manip_utils.h"
#include "plugin_package_manip_res_collector_i.h"

static int plugin_package_manip_res_collector_extern_shared(void * ctx, plugin_package_package_t package, cfg_t res_cfg, cfg_t package_cfg) {
    return plugin_package_manip_collect_extern_shared(package);    
}

int plugin_package_manip_create_res_collector_extern_shared(plugin_package_manip_t manip) {
    plugin_package_manip_res_collector_t collector =
        plugin_package_manip_res_collector_create(
            manip, "extern-shared", plugin_package_manip_res_collector_extern_shared, manip);
    if (collector == NULL) return -1;
    return 0;
}
