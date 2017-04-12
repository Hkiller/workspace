#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/file.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_dir.h"
#include "gd/app/app_context.h"
#include "plugin_package_module_i.h"
#include "plugin_package_package_i.h"
#include "plugin_package_region_i.h"
#include "plugin_package_group_i.h"

int plugin_package_module_install_summary(plugin_package_module_t module) {
    cfg_t summary_cfg;
    vfs_mgr_t vfs = gd_app_vfs_mgr(module->m_app);
    struct cfg_it region_it;
    cfg_t region_cfg;
    int rv = 0;
    
    summary_cfg = cfg_create(module->m_alloc);
    if (summary_cfg == NULL) {
        CPE_ERROR(module->m_em, "plugin_package_install: create summary fail!");
        return -1;
    }

    if (module->m_repo_path) {
        const char * repo_summary_path;
        
        if (mem_buffer_printf(gd_app_tmp_buffer(module->m_app), "%s/summary.bin", module->m_repo_path) != 0) {
            CPE_ERROR(module->m_em, "plugin_package_install: build repo summary path fail!");
            cfg_free(summary_cfg);
            return -1;
        }

        repo_summary_path = mem_buffer_make_continuous(gd_app_tmp_buffer(module->m_app), 0);
        
        if (vfs_file_exist(vfs, repo_summary_path)) {
            if (cfg_bin_read_file(summary_cfg, vfs, repo_summary_path, module->m_em) != 0) {
                CPE_ERROR(module->m_em, "plugin_package_install: read summary from %s fail!", repo_summary_path);
                cfg_free(summary_cfg);
                return -1;
            }
        }
    }

    if (cfg_child_count(summary_cfg) == 0) {
        if (cfg_bin_read_file(summary_cfg, vfs, "summary.bin", module->m_em) != 0) {
            CPE_ERROR(module->m_em, "plugin_package_install: read summary from summary.bin fail!");
            cfg_free(summary_cfg);
            return -1;
        }
    }

    cfg_it_init(&region_it, summary_cfg);
    while((region_cfg = cfg_it_next(&region_it))) {
        const char * region_name = cfg_name(region_cfg);
        plugin_package_region_t region;
        struct cfg_it package_it;
        cfg_t package_cfg;

        region = plugin_package_region_find(module, region_name);
        if (region == NULL) {
            region = plugin_package_region_create(module, region_name);
            if (region == NULL) {
                CPE_ERROR(module->m_em, "plugin_package_install: create region %s fail!", region_name);
                rv = -1;
                continue;
            }
        }

        cfg_it_init(&package_it, region_cfg);
        while((package_cfg = cfg_it_next(&package_it))) {
            const char * package_name = cfg_get_string(package_cfg, "name", NULL);
            plugin_package_package_t package;
            struct cfg_it base_package_it;
            cfg_t base_package_cfg;
            
            package = plugin_package_package_find(module, package_name);
            if (package == NULL) {
                package = plugin_package_package_create(module, package_name, plugin_package_package_empty);
                if (package == NULL) {
                    CPE_ERROR(module->m_em, "plugin_package_install: region %s: package %s create fail", region_name, package_name);
                    rv = -1;
                    continue;
                }
            }

            cfg_it_init(&base_package_it, cfg_find_cfg(package_cfg, "base-packages"));
            while((base_package_cfg = cfg_it_next(&base_package_it))) {
                const char * base_package_name = cfg_as_string(base_package_cfg, NULL);
                plugin_package_package_t base_package;

                base_package = plugin_package_package_find(module, base_package_name);
                if (base_package == NULL) {
                    base_package = plugin_package_package_create(module, base_package_name, plugin_package_package_empty);
                    if (package == NULL) {
                        CPE_ERROR(
                            module->m_em, "plugin_package_install: region %s: package %s: base package %s create fail",
                            region_name, package_name, base_package_name);
                        rv = -1;
                        continue;
                    }
                }

                if (plugin_package_package_has_base_package(package, base_package)) continue;

                if (plugin_package_package_add_base_package(package, base_package) != 0) {
                    CPE_ERROR(
                        module->m_em, "plugin_package_install: region %s: package %s: add base package %s ",
                        region_name, package_name, base_package_name);
                    continue;
                }
            }

            plugin_package_group_add_package(plugin_package_region_group(region), package);
        }
    }
    
    cfg_free(summary_cfg);

    return rv;
}

int plugin_package_module_install_packages(plugin_package_module_t module) {
    vfs_mgr_t vfs = gd_app_vfs_mgr(module->m_app);
    struct plugin_package_package_it package_it;
    plugin_package_package_t package;
    mem_buffer_t path_buffer = gd_app_tmp_buffer(module->m_app);
    int rv = 0;
    
    plugin_package_module_packages(module, &package_it);
    while((package = plugin_package_package_it_next(&package_it))) {
        char * path = NULL;
        
        if (plugin_package_package_state(package) != plugin_package_package_empty) continue;

        if (module->m_repo_path) {
            mem_buffer_printf(path_buffer, "%s/%s/package.bin", module->m_repo_path, plugin_package_package_name(package));
            path = mem_buffer_make_continuous(path_buffer, 0);
            file_name_normalize(path);
            if (!vfs_file_exist(vfs, path)) path = NULL;
        }

        if (path == NULL) {
            if (gd_app_root(module->m_app)) {
                mem_buffer_printf(path_buffer, "%s/packages/%s/package.bin", gd_app_root(module->m_app), plugin_package_package_name(package));
            }
            else {
                mem_buffer_printf(path_buffer, "packages/%s/package.bin", plugin_package_package_name(package));
            }
            path = mem_buffer_make_continuous(path_buffer, 0);
            file_name_normalize(path);
            if (!vfs_file_exist(vfs, path)) path = NULL;
        }

        if (path == NULL) {
            if (module->m_debug >= 2) {
                CPE_INFO(
                    module->m_em, "plugin_package_module_install_packages: package %s: not download",
                    plugin_package_package_name(package));
            }
            continue;
        }

        * strrchr(path, '/') = 0;

        if (plugin_package_package_set_path(package, path) != 0) {
            CPE_ERROR(module->m_em, "plugin_package_module_install_packages: package %s: set path fail", plugin_package_package_name(package));
            rv = -1;
            continue;
        }

        if (plugin_package_package_install(package) != 0) {
            CPE_ERROR(
                module->m_em, "plugin_package_module_install_packages: package %s: package install fail",
                plugin_package_package_name(package));
            rv = -1;
            continue;
        }
    }
    
    return rv;
}
