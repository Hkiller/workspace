#include <assert.h>
#include "cpe/utils/buffer.h"
#include "cpe/utils/file.h"
#include "cpe/vfs/vfs_dir.h"
#include "cpe/vfs/vfs_file.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin/package/plugin_package_package.h"
#include "plugin/package/plugin_package_group.h"
#include "plugin/package/plugin_package_region.h"
#include "plugin/package_manip/plugin_package_manip.h"
#include "convert_ctx.h"

int convert_save_summary(convert_ctx_t ctx) {
    struct plugin_package_region_it region_it;
    plugin_package_region_t region;
    struct plugin_package_package_it package_it;
    plugin_package_package_t package;
    char path_buf[256];
    cfg_t summary;
    int rv = 0;

    summary = cfg_create(ctx->m_alloc);
    if (summary == NULL) {
        CPE_ERROR(ctx->m_em, "convert_save_summary: create summary fail!");
        return -1;
    }

    plugin_package_module_regions(ctx->m_package_module, &region_it);
    while((region = plugin_package_region_it_next(&region_it))) {
        cfg_t region_cfg;

        region_cfg = cfg_struct_add_seq(summary, plugin_package_region_name(region), cfg_replace);
        if(region_cfg == NULL) {
            CPE_ERROR(ctx->m_em, "convert_save_summary: add region cfg fail!");
            continue;
        }
        
        plugin_package_group_packages(&package_it, plugin_package_region_group(region));
        while((package = plugin_package_package_it_next(&package_it))) {
            cfg_t package_cfg;
            cfg_t base_package_cfg;
            struct plugin_package_package_it base_package_it;
            plugin_package_package_t base_package;

            package_cfg = cfg_seq_add_struct(region_cfg);
            if (package_cfg == NULL) {
                CPE_ERROR(
                    ctx->m_em, "convert_save_summary: region %s: add pakcage %s fail!",
                    plugin_package_region_name(region), plugin_package_package_name(package));
                continue;
            }

            if (cfg_struct_add_string(package_cfg, "name", plugin_package_package_name(package), cfg_replace) == NULL) {
                CPE_ERROR(
                    ctx->m_em, "convert_save_summary: region %s: add pakcage %s set name fail!",
                    plugin_package_region_name(region), plugin_package_package_name(package));
                continue;
            }

            base_package_cfg = cfg_struct_add_seq(package_cfg, "base-packages", cfg_replace);
            if (base_package_cfg == NULL) {
                CPE_ERROR(
                    ctx->m_em, "convert_save_summary: region %s: add pakcage %s base package node fail!",
                    plugin_package_region_name(region), plugin_package_package_name(package));
                continue;
            }

            plugin_package_package_base_packages(package, &base_package_it);
            while((base_package = plugin_package_package_it_next(&base_package_it))) {
                if (cfg_seq_add_string(base_package_cfg, plugin_package_package_name(base_package)) == NULL) {
                    CPE_ERROR(
                        ctx->m_em, "convert_save_summary: region %s: add pakcage %s: add base package %s fail!",
                        plugin_package_region_name(region),
                        plugin_package_package_name(package), plugin_package_package_name(base_package));
                    continue;
                }
            }
        }
    }

    snprintf(path_buf, sizeof(path_buf), "%s/summary.bin", ctx->m_output);
    if (cfg_bin_write_file(summary, gd_app_vfs_mgr(ctx->m_app), path_buf, ctx->m_em) != 0) rv = -1;

    /* snprintf(path_buf, sizeof(path_buf), "%s/summary.yml", ctx->m_output); */
    /* if (cfg_yaml_write_file(summary, gd_app_vfs_mgr(ctx->m_app), path_buf, ctx->m_em) != 0) rv = -1; */
    
    cfg_free(summary);
    
    return rv;
}
