#include <assert.h>
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_language.h"
#include "render/model/ui_data_src_group.h"
#include "render/cache/ui_cache_group.h"
#include "render/cache/ui_cache_texture.h"
#include "plugin_package_package_i.h"
#include "plugin_package_language_i.h"
#include "plugin_package_installer_i.h"

static int plugin_package_package_install_base_package(plugin_package_package_t package, cfg_t src_cfg);
static int plugin_package_package_install_src(plugin_package_package_t package, cfg_t src_cfg);
static int plugin_package_package_install_res(plugin_package_package_t package, cfg_t res_cfg);

int plugin_package_package_install(plugin_package_package_t package) {
    plugin_package_module_t module = package->m_module;
    cfg_t package_cfg;
    struct cfg_it child_it;
    cfg_t child_cfg;

    switch(package->m_state) {
    case plugin_package_package_empty:
        if(package->m_path) break;
        
        /*没有path，代表没有安装到系统中 */
        if (module->m_installer == NULL) {
            CPE_ERROR(module->m_em, "package %s: install: package not installed!", package->m_name);
            return -1;
        }

        package->m_progress = 0.0001; /*代表已经开始安装 */
        plugin_package_package_set_state(package, plugin_package_package_downloading);
        if (module->m_installer->m_start_fun(module->m_installer->m_ctx, package) != 0) {
            CPE_ERROR(module->m_em, "package %s: install: start download fail!", package->m_name);
            return -1;
        }

        switch(package->m_state) {
        case plugin_package_package_empty:
            if(package->m_path) {
                break;
            }
            else {
                CPE_INFO(module->m_em, "package %s: install: start download fail!", package->m_name);
                return -1;
            }
        case plugin_package_package_downloading:
            CPE_INFO(module->m_em, "package %s: install: start download success!", package->m_name);
            return 0;
        default:
            CPE_ERROR(
                module->m_em, "package %s: install: after start download, state is %s, can`t install!",
                package->m_name, plugin_package_package_state_str(package));
            return -1;
        }
        break;
    case plugin_package_package_downloading:
        return 0;
    default:
        CPE_ERROR(
            module->m_em, "package %s: install: state is %s, can`t install!",
            package->m_name, plugin_package_package_state_str(package));
        return -1;
    }

    /*读取配置文件 */
    package_cfg = cfg_create(module->m_alloc);
    if (package_cfg == NULL) {
        CPE_ERROR(module->m_em, "package %s: install: alloc cfg fail", package->m_name);
        return -1;
    }

    mem_buffer_printf(gd_app_tmp_buffer(module->m_app), "%s/package.bin", package->m_path);
    if (cfg_bin_read_file(
            package_cfg,
            gd_app_vfs_mgr(module->m_app),
            mem_buffer_make_continuous(gd_app_tmp_buffer(module->m_app), 0),
            module->m_em) != 0)
    {
        CPE_ERROR(
            module->m_em, "package %s: install: read package summary from %s fail",
            package->m_name, (char*)mem_buffer_make_continuous(gd_app_tmp_buffer(module->m_app), 0));
        cfg_free(package_cfg);
        return -1;
    }

    //printf("xxxx: %s\n", cfg_dump(package_cfg, gd_app_tmp_buffer(module->m_app), 0, 4));
    
    /*根据配置文件构造数据 */
    cfg_it_init(&child_it, cfg_find_cfg(package_cfg, "base-packages"));
    while((child_cfg = cfg_it_next(&child_it))) {
        if (plugin_package_package_install_base_package(package, child_cfg) != 0) return -1;
    }
    
    cfg_it_init(&child_it, cfg_find_cfg(package_cfg, "src-root"));
    while((child_cfg = cfg_it_next(&child_it))) {
        if (plugin_package_package_install_src(package, child_cfg) != 0) return -1;
    }

    cfg_it_init(&child_it, cfg_find_cfg(package_cfg, "resources"));
    while((child_cfg = cfg_it_next(&child_it))) {
        if (plugin_package_package_install_res(package, child_cfg) != 0) return -1;
    }
    
    /*修改最终状态 */
    plugin_package_package_set_state(package, plugin_package_package_installed);
    
    cfg_free(package_cfg);

    if (module->m_debug >= 1) {
        CPE_INFO(module->m_em, "package %s: install success", package->m_name);
    }
    
    return 0;
}

static int plugin_package_package_install_base_package(plugin_package_package_t package, cfg_t base_cfg) {
    plugin_package_module_t module = package->m_module;
    plugin_package_package_t base_package;
    const char * package_name;

    package_name = cfg_as_string(base_cfg, NULL);
    if (package_name == NULL) {
        CPE_ERROR(module->m_em, "package %s: install: base package format error", package->m_name);
        return -1;
    }

    base_package = plugin_package_package_find(module, package_name);
    if (base_package == NULL) {
        base_package = plugin_package_package_create(module, package_name, plugin_package_package_empty);
        if (base_package == NULL) {
            CPE_ERROR(module->m_em, "package %s: install: base package %s create tmp package fail", package->m_name, package_name);
            return -1;
        }
    }

    if (plugin_package_package_has_base_package(package, base_package)) return 0;
    
    if (plugin_package_package_add_base_package(package, base_package) != 0) {
        CPE_ERROR(module->m_em, "package %s: install: base package %s add fail", package->m_name, package_name);
        return -1;
    }

    if (module->m_debug >= 2) {
        CPE_INFO(module->m_em, "package %s: add base package %s", package->m_name, package_name);
    }

    return 0;
}

static int plugin_package_package_install_src(plugin_package_package_t package, cfg_t src_cfg) {
    plugin_package_module_t module = package->m_module;
    const char * src_path;
    ui_data_src_type_t src_type;
    ui_data_src_t child_src;
    uint32_t src_id;
    
    src_path = cfg_get_string(src_cfg, "path", NULL);
    src_type = (ui_data_src_type_t)cfg_get_uint8(src_cfg, "type", 0);

    if (src_path == NULL) {
        CPE_ERROR(module->m_em, "package %s: install: src path not configured", package->m_name);
        return -1;
    }

    child_src = ui_data_src_create_relative(module->m_data_mgr, src_type, src_path);
    if (child_src == NULL) {
        CPE_ERROR(module->m_em, "package %s: install: src %s %s cereate fail", package->m_name, ui_data_src_type_name(src_type), src_path);
        return 0;
    }
           
    src_id = cfg_get_uint32(src_cfg, "id", (uint32_t)-1);
    if (src_id != (uint32_t)-1) {
        if (ui_data_src_set_id(child_src, src_id) != 0) {
            CPE_ERROR(
                module->m_em, "package %s: install: create type %s src %s set id %u fail",
                package->m_name, ui_data_src_type_name(src_type),
                ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), child_src), src_id);
            return -1;
        }
    }

    if (ui_data_src_group_add_src(package->m_srcs, child_src) != 0) {
        CPE_ERROR(
            module->m_em, "package %s: install: add src %s fail",
            package->m_name, ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), child_src));
        ui_data_src_free(child_src);
        return -1;
    }

    if (module->m_debug >= 2) {
        CPE_INFO(
            module->m_em, "package %s: install src %s %s",
            package->m_name,
            ui_data_src_type_name(src_type),
            ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), child_src));
    }
    
    return 0;
}

static int plugin_package_package_install_res(plugin_package_package_t package, cfg_t res_cfg) {
    plugin_package_module_t module = package->m_module;
    uint8_t res_type = cfg_get_uint8(res_cfg, "type", 0);
    const char * res_file = cfg_get_string(res_cfg, "path", NULL);
    const char * language = cfg_get_string(res_cfg, "language", NULL);
    ui_cache_res_t res;

    if (res_file == NULL) {
        CPE_ERROR(module->m_em, "package %s: install: res name not configured", package->m_name);
        return -1;
    }

    res = ui_cache_res_find_by_path(module->m_cache_mgr, res_file);
    if (res != NULL) {
    }
    else {
        res = ui_cache_res_create(module->m_cache_mgr, (ui_cache_res_type_t)res_type);
        if (res == NULL) {
            CPE_ERROR(module->m_em, "package %s: install: create res fail", package->m_name);
            return -1;
        }

        if (ui_cache_res_set_path(res, res_file) != 0) {
            CPE_ERROR(module->m_em, "package %s: install: set res path fail", package->m_name);
            ui_cache_res_free(res);
            return -1;
        }

        if (res_type == ui_cache_res_type_texture) {
            uint32_t width = cfg_get_uint32(res_cfg, "width", 0);
            uint32_t height = cfg_get_uint32(res_cfg, "height", 0);
            if (ui_cache_texture_set_summary(res, width, height, ui_cache_pf_r8g8b8a8) != 0) {
                CPE_ERROR(module->m_em, "package %s: install: set texture summary %d*%d fail", package->m_name, width, height);
                ui_cache_res_free(res);
                return -1;
            }
        }
    }

    if (language) {
        ui_data_language_t data_language;
        plugin_package_language_t package_language;

        data_language = ui_data_language_find(module->m_data_mgr, language);
        if (data_language == NULL) {
            CPE_ERROR(
                module->m_em, "package %s: install: language %s not exist!",
                package->m_name, language);
            ui_cache_res_free(res);
            return -1;
        }

        package_language = plugin_package_language_find(package, data_language);
        if (package_language == NULL) {
            package_language = plugin_package_language_create(package, data_language);
            if (package_language == NULL) {
                CPE_ERROR(
                    module->m_em, "package %s: install: language %s create fail!",
                    package->m_name, language);
                ui_cache_res_free(res);
                return -1;
            }
        }

        if (ui_cache_group_add_res(package_language->m_resources, res) != 0) {
            CPE_ERROR(module->m_em, "package %s: install: add texture to language group fail", package->m_name);
            ui_cache_res_free(res);
            return -1;
        }
    }
    else {
        if (ui_cache_group_add_res(package->m_resources, res) != 0) {
            CPE_ERROR(module->m_em, "package %s: install: add texture to group fail", package->m_name);
            ui_cache_res_free(res);
            return -1;
        }
    }
    
    if (module->m_debug >= 2) {
        CPE_INFO(module->m_em, "package %s: install res %s", package->m_name, ui_cache_res_path(res));
    }

    return 0;
}

int plugin_package_package_uninstall(plugin_package_package_t package) {
    return 0;
}
