#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_calc.h"
#include "gd/app/app_context.h"
#include "plugin/package/plugin_package_region.h"
#include "plugin/package/plugin_package_group.h"
#include "plugin/package_manip/plugin_package_manip_utils.h"
#include "plugin_package_manip_res_collector_i.h"

static int plugin_package_manip_res_collector_region_do(
    void * ctx, plugin_package_package_t package, cfg_t res_cfg, cfg_t package_cfg, cfg_calc_context_t args)
{
    plugin_package_manip_t manip = ctx;
    const char * region_def;
    const char * region_value;
    struct mem_buffer buffer;
    
    region_def = cfg_get_string(res_cfg, "region", NULL);
    if (region_def == NULL) {
        CPE_ERROR(
            manip->m_em, "package: %s: res collect: region: region not configured",
            plugin_package_package_name(package));
        return -1;
    }

    /* mem_buffer_init(&buffer, NULL); */
    /* uint32_t i = 0; */
    /* for(cfg_calc_context_t p = args; p; p = p->m_next) { */
    /*     printf("xxx: arg %d: %s\n", i++, cfg_dump(p->m_cfg, &buffer, 0, 4)); */
    /* } */
    /* mem_buffer_clear(&buffer); */

    mem_buffer_init(&buffer, manip->m_alloc);
    if ((region_value = plugin_package_manip_calc(manip, &buffer, region_def, args)) == NULL) {
        CPE_ERROR(
            manip->m_em, "package: %s: res collect: calc from %s fail",
            plugin_package_package_name(package),  region_def);
        mem_buffer_clear(&buffer);
        return -1;
    }

    if (region_value[0] != 0) {
        plugin_package_region_t region = plugin_package_region_find(manip->m_package_module, region_value);
        if (region == NULL) {
            region = plugin_package_region_create(manip->m_package_module, region_value);
            if (region == NULL) {
                CPE_ERROR(
                    manip->m_em, "package: %s: res collect: package: %s region %s create fail",
                    plugin_package_package_name(package), plugin_package_package_name(package), region_value);
                mem_buffer_clear(&buffer);
                return -1;
            }
        }

        if (plugin_package_group_add_package(plugin_package_region_group(region), package) != 0) {
            CPE_ERROR(
                manip->m_em, "package: %s: res collect: package %s add to region %s fail",
                plugin_package_package_name(package), plugin_package_package_name(package), region_value);
            mem_buffer_clear(&buffer);
            return -1;
        }
    }

    mem_buffer_clear(&buffer);
    return 0;
}

static int plugin_package_manip_res_collector_region(void * ctx, plugin_package_package_t package, cfg_t res_cfg, cfg_t package_cfg) {
    plugin_package_manip_t manip = ctx;
    const char * search;

    if ((search = cfg_get_string(res_cfg, "search", NULL))) {
        struct cfg_calc_context args = { package_cfg, NULL };
        return plugin_package_manip_collect_search(
            manip, package, res_cfg, package_cfg, &args,
            search, plugin_package_manip_res_collector_region_do, manip);
    }
    else {
        struct cfg_calc_context calc_ctx;
        calc_ctx.m_cfg = package_cfg;
        calc_ctx.m_next = NULL;
        return plugin_package_manip_res_collector_region_do(ctx, package, res_cfg, package_cfg, &calc_ctx);
    }
}

int plugin_package_manip_create_res_collector_region(plugin_package_manip_t manip) {
    plugin_package_manip_res_collector_t collector =
        plugin_package_manip_res_collector_create(
            manip, "region", plugin_package_manip_res_collector_region, manip);
    if (collector == NULL) return -1;
    return 0;
}
