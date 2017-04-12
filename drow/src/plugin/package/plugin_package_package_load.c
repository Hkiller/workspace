#include <assert.h>
#include "cpe/utils/time_utils.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_language.h"
#include "render/model/ui_data_src_group.h"
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_group.h"
#include "plugin_package_package_i.h"
#include "plugin_package_load_task_i.h"
#include "plugin_package_group_i.h"
#include "plugin_package_language_i.h"
#include "plugin_package_depend_i.h"

static int plugin_package_package_collect_load_srcs(plugin_package_package_t package, ui_data_src_group_t group);
static void plugin_package_package_unload_srcs(plugin_package_package_t package, ui_data_src_group_t group);
static void plugin_package_package_collect_load_resources(plugin_package_package_t package, ui_cache_group_t group);
static void plugin_package_package_unload_resources(plugin_package_package_t package, ui_cache_group_t group);

int plugin_package_package_load_async(plugin_package_package_t package, plugin_package_load_task_t task) {
    plugin_package_module_t module = package->m_module;

    switch(package->m_state) {
    case plugin_package_package_empty:
    case plugin_package_package_downloading:
        if (module->m_to_download_packages == NULL) {
            module->m_to_download_packages = plugin_package_group_create(module, "to-download");
            if (module->m_to_download_packages == NULL) {
                CPE_ERROR(module->m_em, "package %s: load: create to-download group fail!", package->m_name);
                return -1;
            }
        }
        if (!plugin_package_package_is_in_group(package, module->m_to_download_packages)) {
            if (plugin_package_group_add_package(module->m_to_download_packages, package) != 0) {
                CPE_ERROR(module->m_em, "package %s: load: add package to to-download group fail!", package->m_name);
                return -1;
            }
            module->m_total_download_count++;
            module->m_total_load_count++;
        }
        if (task) plugin_package_load_task_add_package(task, package);
        return 0;
    case plugin_package_package_installed:
    case plugin_package_package_loading:
        if (module->m_to_load_packages == NULL) {
            module->m_to_load_packages = plugin_package_group_create(module, "to-download");
            if (module->m_to_load_packages == NULL) {
                CPE_ERROR(module->m_em, "package %s: load: create to-download group fail!", package->m_name);
                return -1;
            }
        }
        if (!plugin_package_package_is_in_group(package, module->m_to_load_packages)) {
            if (plugin_package_group_add_package(module->m_to_load_packages, package) != 0) {
                CPE_ERROR(module->m_em, "package %s: load: add package to to-download group fail!", package->m_name);
                return -1;
            }
            module->m_total_load_count++;
        }
        if (task) plugin_package_load_task_add_package(task, package);
        return 0;
    case plugin_package_package_loaded:
        return 0;
    default:
        CPE_ERROR(
            module->m_em, "package %s: load: state is %s, can`t load!",
            package->m_name, plugin_package_package_state_str(package));
        return -1;
    }
}

int plugin_package_package_load_async_start(plugin_package_package_t package) {
    plugin_package_module_t module = package->m_module;
    
    switch(package->m_state) {
    case plugin_package_package_installed:
        /*首次加载，转换加载状态 */

        assert(package->m_loading_res_group == NULL);
        package->m_loading_res_group = ui_cache_group_create(module->m_cache_mgr);
        if (package->m_loading_res_group == NULL) {
            CPE_ERROR(module->m_em, "package %s: load tick: create loading res group fail!", package->m_name);
            return -1;
        }
        
        assert(package->m_loading_src_group == NULL);
        package->m_loading_src_group = ui_data_src_group_create(module->m_data_mgr);
        if (package->m_loading_src_group == NULL) {
            CPE_ERROR(module->m_em, "package %s: load tick: create loading src group fail!", package->m_name);
            ui_cache_group_free(package->m_loading_res_group);
            package->m_loading_res_group = NULL;
            return -1;
        }
            
        assert(package->m_active_language == NULL);
        package->m_active_language = plugin_package_language_find(package, ui_data_active_language(module->m_data_mgr));

        /*收集Src */
        plugin_package_package_collect_load_srcs(package, package->m_srcs);
        if (package->m_active_language) {
            plugin_package_package_collect_load_srcs(package, package->m_active_language->m_srcs);
        }
        
        /*收集资源 */
        plugin_package_package_collect_load_resources(package, package->m_resources);
        if (package->m_active_language) {
            plugin_package_package_collect_load_resources(package, package->m_active_language->m_resources);
        }
        
        plugin_package_package_set_state(package, plugin_package_package_loading);
        break;
    case plugin_package_package_loading:
    case plugin_package_package_loaded:
        return 0;
    default:
        CPE_ERROR(
            module->m_em, "package %s: load: state is %s, can`t load!",
            package->m_name, plugin_package_package_state_str(package));
        return -1;
    }

    return 0;
}

int plugin_package_package_load_async_tick(plugin_package_package_t package, int64_t start_time) {
    plugin_package_module_t module = package->m_module;
    int rv = 0;
    
    /*加载src */
    if (package->m_loading_src_group) {
        struct ui_data_src_it src_it;
        ui_data_src_t src;

        if (ui_data_mgr_set_root(module->m_data_mgr, package->m_path) != 0) rv = -1;
        
        ui_data_src_group_srcs(&src_it, package->m_loading_src_group);
        while((src = ui_data_src_it_next(&src_it))) {
            if (!ui_data_src_is_loaded(src)) {
                if (ui_data_src_load(src, module->m_em) != 0) {
                    CPE_ERROR(
                        module->m_em, "package %s: load: src %s %s load fail!",
                        package->m_name,
                        ui_data_src_type_name(ui_data_src_type(src)),
                        ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
                }
            }

            ui_data_src_group_remove_src(package->m_loading_src_group, src);

            if (cur_time_ms() - start_time > module->m_process_tick_limit_ms) break;
        }
        
        ui_data_mgr_set_root(module->m_data_mgr, "");

        ui_data_src_group_srcs(&src_it, package->m_loading_src_group);
        if (ui_data_src_it_next(&src_it) == NULL) {
            ui_data_src_group_free(package->m_loading_src_group);
            package->m_loading_src_group = NULL;
        }
    }

    /*加载res */
    if (package->m_loading_res_group) {
        struct ui_cache_res_it res_it;
        ui_cache_res_t res;
        
        ui_cache_group_using_resources(&res_it, package->m_loading_res_group);
        while((res = ui_cache_res_it_next(&res_it))) {
            switch(ui_cache_res_load_state(res)) {
            case ui_cache_res_not_load:
                if (ui_cache_res_load_async(res, package->m_path) != 0) {
                    CPE_ERROR(module->m_em, "package %s: load: res %s load async fail!", package->m_name, ui_cache_res_path(res));
                    ui_cache_group_remove_res(package->m_loading_res_group, res);
                    break;
                }
                break;
            case ui_cache_res_wait_load:
            case ui_cache_res_loading:
            case ui_cache_res_data_loaded:
                continue;
            case ui_cache_res_loaded:
                //if (module->m_debug >= 2) {
                    CPE_INFO(module->m_em, "package %s: load: res %s: async load success!", package->m_name, ui_cache_res_path(res));
                    //}
                ui_cache_group_remove_res(package->m_loading_res_group, res);
                break;
            case ui_cache_res_load_fail:
                CPE_ERROR(module->m_em, "package %s: load: res %s: async load fail!", package->m_name, ui_cache_res_path(res));
                ui_cache_group_remove_res(package->m_loading_res_group, res);
                break;
            case ui_cache_res_cancel_loading:
                CPE_INFO(module->m_em, "package %s: load: res %s: async load cancel!", package->m_name, ui_cache_res_path(res));
                ui_cache_group_remove_res(package->m_loading_res_group, res);
                break;
            }

            if (cur_time_ms() - start_time > module->m_process_tick_limit_ms) break;
        }

        ui_cache_group_using_resources(&res_it, package->m_loading_res_group);
        if (ui_cache_res_it_next(&res_it) == NULL) {
            ui_cache_group_free(package->m_loading_res_group);
            package->m_loading_res_group = NULL;
        }
    }
        
    if (package->m_loading_res_group == NULL && package->m_loading_src_group == NULL) {
        plugin_package_package_set_state(package, plugin_package_package_loaded);
        if (module->m_debug) {
            CPE_INFO(module->m_em, "package %s: load success(async)", package->m_name);
        }
    }
    
    return rv;
}

int plugin_package_package_load_sync(plugin_package_package_t package) {
    plugin_package_module_t module = package->m_module;
    int rv;

    if(package->m_state == plugin_package_package_loaded) return 0;

    rv = plugin_package_package_load_async_start(package);
    if (rv) return rv;

    if (package->m_loading_src_group) {
        struct ui_data_src_it src_it;
        ui_data_src_t src;

        if (ui_data_mgr_set_root(module->m_data_mgr, package->m_path) != 0) rv = -1;
        
        ui_data_src_group_srcs(&src_it, package->m_loading_src_group);
        while((src = ui_data_src_it_next(&src_it))) {
            if (ui_data_src_is_loaded(src)) continue;
        
            if (ui_data_src_load(src, module->m_em) != 0) {
                CPE_ERROR(
                    module->m_em, "package %s: load: src %s %s load fail!",
                    package->m_name,
                    ui_data_src_type_name(ui_data_src_type(src)),
                    ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
            }
        }

        ui_data_mgr_set_root(module->m_data_mgr, "");
        
        ui_data_src_group_free(package->m_loading_src_group);
        package->m_loading_src_group = NULL;
    }

    if (package->m_loading_res_group) {
        ui_cache_res_t res;

        while((res = ui_cache_group_first(package->m_loading_res_group))) {
            if (ui_cache_res_load_sync(res, package->m_path) != 0) {
                CPE_ERROR(module->m_em, "package %s: load: res %s load fail!", package->m_name, ui_cache_res_path(res));
            }
            ui_cache_group_remove_res(package->m_loading_res_group, res);
        }

        ui_cache_group_free(package->m_loading_res_group);
        package->m_loading_res_group = NULL;
    }
    
    plugin_package_package_set_state(package, plugin_package_package_loaded);
    if (module->m_debug) {
        CPE_INFO(module->m_em, "package %s: load success(sync)", package->m_name);
    }
    
    return 0;
}

int plugin_package_package_unload(plugin_package_package_t package) {
    plugin_package_module_t module = package->m_module;
    
    switch(package->m_state) {
    case plugin_package_package_loaded:
    case plugin_package_package_loading:
        break;
    case plugin_package_package_installed:
        return 0;
    default:
        CPE_ERROR(
            module->m_em, "package %s: unload: state is %s, can`t unload!",
            package->m_name, plugin_package_package_state_str(package));
        return -1;
    }

    /*卸载 */
    plugin_package_package_unload_srcs(package, package->m_srcs);
    if (package->m_active_language) {
        plugin_package_package_unload_srcs(package, package->m_active_language->m_srcs);
    }

    plugin_package_package_unload_resources(package, package->m_resources);
    if (package->m_active_language) {
        plugin_package_package_unload_resources(package, package->m_active_language->m_resources);
    }

    /*清理队列 */
    if (package->m_loading_res_group) {
        ui_cache_group_free(package->m_loading_res_group);
        package->m_loading_res_group = NULL;
    }
    
    if (package->m_loading_src_group) {
        ui_data_src_group_free(package->m_loading_src_group);
        package->m_loading_src_group = NULL;
    }

    /*清理语言 */
    package->m_active_language = NULL;
    
    plugin_package_package_set_state(package, plugin_package_package_installed);
    if (module->m_debug) {
        CPE_INFO(module->m_em, "package %s: unload success", package->m_name);
    }

    return 0;
}

int plugin_package_package_load_async_r(plugin_package_package_t package, plugin_package_load_task_t task) {
    int rv = 0;
    plugin_package_depend_t dep;
    
    if(package->m_state ==  plugin_package_package_loading
       || package->m_state ==  plugin_package_package_loaded) return 0;

    if (plugin_package_package_load_async(package, task) != 0) return -1;

    TAILQ_FOREACH(dep, &package->m_base_packages, m_next_for_extern) {
        if (plugin_package_package_load_async_r(dep->m_base_package, task) != 0) rv = -1;
    }
    
    return rv;
}

int plugin_package_package_load_sync_r(plugin_package_package_t package) {
    int rv = 0;
    plugin_package_depend_t dep;

    if(package->m_state != plugin_package_package_loaded) {
        if (plugin_package_package_load_sync(package) != 0) rv = -1;
    }

    TAILQ_FOREACH(dep, &package->m_base_packages, m_next_for_extern) {
        if (plugin_package_package_load_sync_r(dep->m_base_package) != 0) rv = -1;
    }
    
    return rv;
}

static int plugin_package_package_collect_load_srcs(plugin_package_package_t package, ui_data_src_group_t group) {
    struct ui_data_src_it src_it;
    ui_data_src_t src;
    int rv = 0;
    
    ui_data_src_group_srcs(&src_it, group);
    while((src = ui_data_src_it_next(&src_it))) {
        if (ui_data_src_is_loaded(src)) continue;
        if (ui_data_src_group_add_src(package->m_loading_src_group, src) != 0) rv = -1;
    }
    
    return rv;
}

static void plugin_package_package_unload_srcs(plugin_package_package_t package, ui_data_src_group_t group) {
    struct ui_data_src_it src_it;
    ui_data_src_t src;
    
    ui_data_src_group_srcs(&src_it, group);
    while((src = ui_data_src_it_next(&src_it))) {
        if (!ui_data_src_is_loaded(src)) continue;
        
        ui_data_src_unload(src);
    }
}

static void plugin_package_package_collect_load_resources(plugin_package_package_t package, ui_cache_group_t group) {
    plugin_package_module_t module = package->m_module;
    struct ui_cache_res_it res_it;
    ui_cache_res_t res;

    ui_cache_group_using_resources(&res_it, group);
    while((res = ui_cache_res_it_next(&res_it))) {
        if (module->m_is_control_res) ui_cache_res_ref_inc(res);

        ui_cache_group_add_res(package->m_loading_res_group, res);
    }
}

static void plugin_package_package_unload_resources(plugin_package_package_t package, ui_cache_group_t group) {
    struct ui_cache_res_it res_it;
    ui_cache_res_t res;

    if (package->m_module->m_is_control_res) {
        ui_cache_group_using_resources(&res_it, group);
        while((res = ui_cache_res_it_next(&res_it))) {
            ui_cache_res_ref_dec(res);
            if (ui_cache_res_ref_count(res) == 0) {
                ui_cache_res_unload(res, 0);
            }
        }
    }
}
