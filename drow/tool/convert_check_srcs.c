#include "cpe/utils/buffer.h"
#include "render/model/ui_data_src_group.h"
#include "plugin/package/plugin_package_module.h"
#include "convert_ctx.h"
#include "convert_language.h"

static int convert_check_srcs_r(convert_ctx_t ctx, ui_data_src_t src) {
    int rv = 0;
    
    if (ui_data_src_type(src) == ui_data_src_type_dir) {
        struct ui_data_src_it child_it;
        ui_data_src_t child;
        ui_data_src_childs(&child_it, src);

        while((child = ui_data_src_it_next(&child_it))) {
            if (convert_check_srcs_r(ctx, child) != 0) rv = -1;
        }
    }
    else {
        struct plugin_package_package_it package_it;
        plugin_package_package_t package;
        plugin_package_package_t used_package = NULL;
        
        plugin_package_module_packages(ctx->m_package_module, &package_it);
        while((package = plugin_package_package_it_next(&package_it))) {
            if (!ui_data_src_in_group(src, plugin_package_package_srcs(package))) continue;
            used_package = package;
            break;
        }

        if (used_package == NULL && !ui_data_src_in_group(src, ctx->m_ignore_srcs) && !convert_ctx_is_src_ignore(ctx, src)) {
            CPE_INFO(
                ctx->m_em, "src %s type %s not used!",
                ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src), ui_data_src_type_name(ui_data_src_type(src)));

            if (ui_data_src_collect_ress(src, ctx->m_ignore_textures) != 0) {
                CPE_ERROR(
                    ctx->m_em, "src %s type %s not used, collect dep textures to ignore gexture fail!",
                    ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src), ui_data_src_type_name(ui_data_src_type(src)));
                rv = -1;
            }
        }
    }

    return rv;
}

int convert_check_srcs(convert_ctx_t ctx) {
    struct ui_data_language_it data_language_it;
    ui_data_language_t data_language;
    int rv = 0;
    
    ui_data_languages(&data_language_it, ctx->m_data_mgr);
    while((data_language = ui_data_language_it_next(&data_language_it))) {
        struct ui_data_src_it src_it;
        ui_data_src_t src;
        
        if (convert_language_find(ctx, data_language) != NULL) continue;

        ui_data_language_srcs(&src_it, data_language);
        while((src = ui_data_src_it_next(&src_it))) {
            ui_data_src_group_add_src(ctx->m_ignore_srcs, src);
        }
    }
    
    ui_data_src_group_load_all(ctx->m_ignore_srcs);
    if (convert_check_srcs_r(ctx, ui_data_mgr_src_root(ctx->m_data_mgr)) != 0) rv = -1;

    return rv;
}

