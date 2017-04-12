#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_group.h"
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_group.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin/package/plugin_package_package.h"
#include "convert_ctx.h"

int convert_check_ignores(convert_ctx_t ctx) {
    struct plugin_package_package_it package_it;
    plugin_package_package_t package;
    int rv = 0;

    plugin_package_module_packages(ctx->m_package_module, &package_it);
    while((package = plugin_package_package_it_next(&package_it))) {
        struct ui_data_src_it src_it;
        ui_data_src_t src;
        struct ui_cache_res_it res_it;
        ui_cache_res_t res;

        ui_data_src_group_srcs(&src_it, plugin_package_package_srcs(package));
        while((src = ui_data_src_it_next(&src_it))) {
            if (convert_ctx_is_src_ignore(ctx, src)) {
                CPE_ERROR(
                    ctx->m_em, "package %s used ignore src %s %s",
                    plugin_package_package_name(package),
                    ui_data_src_type_name(ui_data_src_type(src)),
                    ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src));
                rv = -1;
            }
        }

        ui_cache_group_using_resources(&res_it, plugin_package_package_resources(package));
        while((res = ui_cache_res_it_next(&res_it))) {
            if (convert_ctx_is_res_ignore(ctx, res)) {
                CPE_ERROR(
                    ctx->m_em, "package %s used ignore resource %s",
                    plugin_package_package_name(package), ui_cache_res_path(res));
                rv = -1;
            }
        }
    }

    return rv;
}

