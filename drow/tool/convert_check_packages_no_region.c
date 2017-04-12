#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin/package/plugin_package_group.h"
#include "plugin/package/plugin_package_region.h"
#include "plugin/package/plugin_package_package.h"
#include "convert_ctx.h"

int convert_check_packages_no_region(convert_ctx_t ctx) {
    struct plugin_package_package_it package_it;
    plugin_package_package_t package;
    int rv = 0;
    struct plugin_package_region_it region_it;
    plugin_package_region_t region;
    plugin_package_group_t packages_in_region;

    packages_in_region = plugin_package_group_create(ctx->m_package_module, "packages_in_region");
    if (packages_in_region == NULL) {
        CPE_ERROR(ctx->m_em, "convert_check_packages_no_region: create packages_in_region group fail");
        return -1;
    }

    plugin_package_module_regions(ctx->m_package_module, &region_it);
    while((region = plugin_package_region_it_next(&region_it))) {
        if (plugin_package_group_add_packages(packages_in_region, plugin_package_region_group(region)) != 0) rv = -1;
    }
    
    plugin_package_module_packages(ctx->m_package_module, &package_it);
    while((package = plugin_package_package_it_next(&package_it))) {
        if (plugin_package_package_is_in_group(package, packages_in_region)) continue;

        CPE_ERROR(ctx->m_em, "package %s: not in any region", plugin_package_package_name(package));
        rv = -1;
    }

    plugin_package_group_free(packages_in_region);

    return rv;
}

