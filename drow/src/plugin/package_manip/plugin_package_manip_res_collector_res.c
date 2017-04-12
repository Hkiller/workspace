#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_calc.h"
#include "gd/app/app_context.h"
#include "render/cache/ui_cache_group.h"
#include "render/cache/ui_cache_res.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin/package/plugin_package_package.h"
#include "plugin/package_manip/plugin_package_manip_utils.h"
#include "plugin_package_manip_res_collector_i.h"

static int plugin_package_manip_res_collector_res_do(
    void * ctx, plugin_package_package_t package, cfg_t res_cfg, cfg_t package_cfg, cfg_calc_context_t args)
{
    plugin_package_manip_t manip = ctx;
    const char * res_def;
    const char * res_value;
    struct mem_buffer buffer;
    int rv = 0;

    mem_buffer_init(&buffer, manip->m_alloc);
    
    res_def = cfg_get_string(res_cfg, "res", NULL);
    if (res_def == NULL) {
        CPE_ERROR(
            manip->m_em, "package: %s: res collect: res: res not configured",
            plugin_package_package_name(package));
        rv = -1;
        goto COLLECT_COMPLETE;
    }

    if ((res_value = plugin_package_manip_calc(manip, &buffer, res_def, args)) == NULL) {
        CPE_ERROR(
            manip->m_em, "package: %s: res collect: calc from %s fail",
            plugin_package_package_name(package),  res_def);
        rv = -1;
        goto COLLECT_COMPLETE;
    }

    if (res_value[0]) {
        if (ui_cache_group_add_res_by_path(plugin_package_package_resources(package), res_value) != 0) {
            CPE_ERROR(
                manip->m_em, "package: %s: res collect: res: collect res from res %s fail",
                plugin_package_package_name(package), res_value);
            rv = -1;
            goto COLLECT_COMPLETE;
        }
    }
    
COLLECT_COMPLETE:
    mem_buffer_clear(&buffer);
    return rv;
}

static int plugin_package_manip_res_collector_res(void * ctx, plugin_package_package_t package, cfg_t res_cfg, cfg_t package_cfg) {
    plugin_package_manip_t manip = ctx;
    const char * search;
    
    if ((search = cfg_get_string(res_cfg, "search", NULL))) {
        struct cfg_calc_context args = { package_cfg, NULL };
        return plugin_package_manip_collect_search(
            manip, package, res_cfg, package_cfg, &args,
            search, plugin_package_manip_res_collector_res_do, manip);
    }
    else {
        struct cfg_calc_context calc_ctx;
        calc_ctx.m_cfg = package_cfg;
        calc_ctx.m_next = NULL;
        return plugin_package_manip_res_collector_res_do(ctx, package, res_cfg, package_cfg, &calc_ctx);
    }
}

int plugin_package_manip_create_res_collector_res(plugin_package_manip_t manip) {
    plugin_package_manip_res_collector_t collector =
        plugin_package_manip_res_collector_create(
            manip, "res", plugin_package_manip_res_collector_res, manip);
    if (collector == NULL) return -1;
    return 0;
}
