#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_group.h"
#include "plugin/package/plugin_package_module.h"
#include "convert_ctx.h"
#include "convert_language.h"

int convert_check_resources(convert_ctx_t ctx) {
    struct ui_cache_res_it res_it;
    ui_cache_res_t res;
    int rv = 0;
    
    ui_cache_manager_ress(ctx->m_cache_mgr, &res_it);
    while((res = ui_cache_res_it_next(&res_it))) {
        struct plugin_package_package_it package_it;
        plugin_package_package_t package;
        plugin_package_package_t used_package = NULL;

        if (strcmp(ui_cache_res_path(res), "") == 0) continue;

        plugin_package_module_packages(ctx->m_package_module, &package_it);
        while((package = plugin_package_package_it_next(&package_it))) {
            if (ui_cache_res_in_group(res, plugin_package_package_resources(package))) {
                used_package = package;
                break;
            }
        }

        if (used_package == NULL
            && !ui_cache_res_in_group(res, ctx->m_ignore_textures)
            && !convert_ctx_is_res_ignore(ctx, res))
        {
            CPE_INFO(ctx->m_em, "res %s type %s not used!", ui_cache_res_path(res), ui_cache_res_type_to_str(ui_cache_res_type(res)));
        }
    }

    return rv;
}

