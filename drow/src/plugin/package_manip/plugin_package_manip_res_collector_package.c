#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_calc.h"
#include "gd/app/app_context.h"
#include "plugin/package_manip/plugin_package_manip_utils.h"
#include "plugin_package_manip_res_collector_i.h"

static int plugin_package_manip_res_collector_package_do(
    void * ctx, plugin_package_package_t package, cfg_t res_cfg, cfg_t package_cfg, cfg_calc_context_t args)
{
    plugin_package_manip_t manip = ctx;
    const char * package_def;
    const char * package_value;
    struct mem_buffer buffer;
    
    package_def = cfg_get_string(res_cfg, "package", NULL);
    if (package_def == NULL) {
        CPE_ERROR(
            manip->m_em, "package: %s: res collect: package: package not configured",
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
    if ((package_value = plugin_package_manip_calc(manip, &buffer, package_def, args)) == NULL) {
        CPE_ERROR(
            manip->m_em, "package: %s: res collect: calc from %s fail",
            plugin_package_package_name(package),  package_def);
        mem_buffer_clear(&buffer);
        return -1;
    }

    if (package_value[0] != 0) {
        plugin_package_package_t base_package = plugin_package_package_find(manip->m_package_module, package_value);
        if (base_package == NULL) {
            const char * base_name_sep = strchr(package_value, '/');
            const char * self_name_sep = strchr(plugin_package_package_name(package), '/');
            
            if (base_name_sep && self_name_sep
                && (base_name_sep - package_value) == (self_name_sep - plugin_package_package_name(package))
                && memcmp(plugin_package_package_name(package), package_value, base_name_sep - package_value) == 0)
            {
                base_package = plugin_package_package_create(manip->m_package_module, package_value, plugin_package_package_loaded);
                if (base_package == NULL) {
                    CPE_ERROR(
                        manip->m_em, "package: %s: res collect: package: %s base %s create fail",
                        plugin_package_package_name(package), plugin_package_package_name(package), package_value);
                    mem_buffer_clear(&buffer);
                    return -1;
                }
            }
            else {
                CPE_ERROR(
                    manip->m_em, "package: %s: res collect: package: %s base %s not exist",
                    plugin_package_package_name(package), plugin_package_package_name(package), package_value);
                mem_buffer_clear(&buffer);
                return -1;
            }
        }

        if (plugin_package_package_add_base_package(package, base_package) != 0) {
            CPE_ERROR(
                manip->m_em, "package: %s: res collect: package %s add base %s fail",
                plugin_package_package_name(package), plugin_package_package_name(package), package_value);
            mem_buffer_clear(&buffer);
            return -1;
        }
    }

    mem_buffer_clear(&buffer);
    return 0;
}

static int plugin_package_manip_res_collector_package(void * ctx, plugin_package_package_t package, cfg_t res_cfg, cfg_t package_cfg) {
    plugin_package_manip_t manip = ctx;
    const char * search;

    if ((search = cfg_get_string(res_cfg, "search", NULL))) {
        struct cfg_calc_context args = { package_cfg, NULL };
        return plugin_package_manip_collect_search(
            manip, package, res_cfg, package_cfg, &args,
            search, plugin_package_manip_res_collector_package_do, manip);
    }
    else {
        struct cfg_calc_context calc_ctx;
        calc_ctx.m_cfg = package_cfg;
        calc_ctx.m_next = NULL;
        return plugin_package_manip_res_collector_package_do(ctx, package, res_cfg, package_cfg, &calc_ctx);
    }
}

int plugin_package_manip_create_res_collector_package(plugin_package_manip_t manip) {
    plugin_package_manip_res_collector_t collector =
        plugin_package_manip_res_collector_create(
            manip, "package", plugin_package_manip_res_collector_package, manip);
    if (collector == NULL) return -1;
    return 0;
}
