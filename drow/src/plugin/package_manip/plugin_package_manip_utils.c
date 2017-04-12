#include "cpe/utils/string_utils.h"
#include "cpe/utils/buffer.h"
#include "render/model/ui_data_src_group.h"
#include "render/model/ui_data_src.h"
#include "render/cache/ui_cache_group.h"
#include "render/cache/ui_cache_res.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin/package/plugin_package_package.h"
#include "plugin/package/plugin_package_group.h"
#include "plugin/package_manip/plugin_package_manip_utils.h"
#include "plugin_package_manip_i.h"

int plugin_package_manip_collect_src_by_res(ui_data_src_group_t group, const char * res) {
    if (res == NULL) return 0;
    if (res[0] == '@' || res[0] == ':') return 0;
    return ui_data_src_group_add_src_by_res(group, res);
}

int plugin_package_manip_collect_extern_shared(plugin_package_package_t base_package) {
    plugin_package_module_t package_module = plugin_package_package_module(base_package);
    ui_data_mgr_t data_mgr = plugin_package_module_data_mgr(package_module);
    ui_cache_manager_t cache_mgr = plugin_package_module_cache_mgr(package_module);
    struct plugin_package_package_it package_it;
    plugin_package_package_t package;
    ui_data_src_group_t checked_srcs = ui_data_src_group_create(data_mgr);
    ui_data_src_group_t duplicate_srcs = ui_data_src_group_create(data_mgr);
    ui_cache_group_t checked_resources = ui_cache_group_create(cache_mgr);
    ui_cache_group_t duplicate_resources = ui_cache_group_create(cache_mgr);
    struct ui_data_src_it src_it;
    ui_data_src_t src;
    struct ui_cache_res_it res_it;
    ui_cache_res_t res;
    int rv = 0;

    /*查找所有重复的资源 */
    plugin_package_package_extern_packages(base_package, &package_it);
    while((package = plugin_package_package_it_next(&package_it))) {
        /*处理重复的src */
        ui_data_src_group_srcs(&src_it, plugin_package_package_srcs(package));
        while((src = ui_data_src_it_next(&src_it))) {
            if (!ui_data_src_in_group(src, checked_srcs)) {
                ui_data_src_group_add_src(checked_srcs, src);
            }
            else {
                ui_data_src_group_add_src(duplicate_srcs, src);
            }
        }

        /*处理重复的资源 */
        ui_cache_group_using_resources(&res_it, plugin_package_package_resources(package));
        while((res = ui_cache_res_it_next(&res_it))) {
            if (!ui_cache_res_in_group(res, checked_resources)) {
                if (ui_cache_group_add_res(checked_resources, res) != 0) rv = -1;
            }
            else {
                if (ui_cache_group_add_res(duplicate_resources, res) != 0) rv = -1;
            }
        }        
    }

    /*移除所有重复的资源 */
    ui_data_src_group_srcs(&src_it, duplicate_srcs);
    while((src = ui_data_src_it_next(&src_it))) {
        plugin_package_package_extern_packages(base_package, &package_it);
        while((package = plugin_package_package_it_next(&package_it))) {
            ui_data_src_group_remove_src(plugin_package_package_srcs(package), src);
        }        
    }

    ui_cache_group_using_resources(&res_it, duplicate_resources);
    while((res = ui_cache_res_it_next(&res_it))) {
        plugin_package_package_extern_packages(base_package, &package_it);
        while((package = plugin_package_package_it_next(&package_it))) {
            ui_cache_group_remove_res(plugin_package_package_resources(package), res);
        }        
    }

    /*将所有重复资源添加到包中 */
    ui_data_src_group_srcs(&src_it, duplicate_srcs);
    while((src = ui_data_src_it_next(&src_it))) {
        if (ui_data_src_group_add_src(plugin_package_package_srcs(base_package), src) != 0) rv = -1;
    }

    ui_cache_group_using_resources(&res_it, duplicate_resources);
    while((res = ui_cache_res_it_next(&res_it))) {
        if (ui_cache_group_add_res(plugin_package_package_resources(base_package), res) != 0) rv = -1;
    }     

    /*释放临时缓存 */
    ui_data_src_group_free(checked_srcs);
    ui_data_src_group_free(duplicate_srcs);
    ui_cache_group_free(checked_resources);
    ui_cache_group_free(duplicate_resources);
    
    return rv;
}

uint8_t plugin_package_manip_src_is_base_provided(ui_data_src_t src, plugin_package_package_t package) {
    struct plugin_package_package_it base_package_it;
    plugin_package_package_t base_package;
    
    plugin_package_package_base_packages(package, &base_package_it);
    while((base_package = plugin_package_package_it_next(&base_package_it))) {
        if (ui_data_src_in_group(src, plugin_package_package_srcs(base_package))) return 1;
        if (plugin_package_manip_src_is_base_provided(src, base_package)) return 1;
    }

    return 0;
}

uint8_t plugin_package_manip_res_is_base_provided(ui_cache_res_t res, plugin_package_package_t package) {
    struct plugin_package_package_it base_package_it;
    plugin_package_package_t base_package;
    
    plugin_package_package_base_packages(package, &base_package_it);
    while((base_package = plugin_package_package_it_next(&base_package_it))) {
        if (ui_cache_res_in_group(res, plugin_package_package_resources(base_package))) {
            //printf("     res %s found in %s\n", ui_cache_res_path(res), plugin_package_package_name(base_package));
            return 1;
        }
        
        if (plugin_package_manip_res_is_base_provided(res, base_package)) return 1;
    }

    return 0;
}

int plugin_package_manip_remove_base_provided(plugin_package_package_t package) {
    plugin_package_module_t package_module = plugin_package_package_module(package);
    ui_data_mgr_t data_mgr = plugin_package_module_data_mgr(package_module);
    ui_cache_manager_t cache_mgr = plugin_package_module_cache_mgr(package_module);
    ui_data_src_group_t remove_srcs = ui_data_src_group_create(data_mgr);
    ui_cache_group_t remove_resources = ui_cache_group_create(cache_mgr);
    struct ui_data_src_it src_it;
    ui_data_src_t src;
    struct ui_cache_res_it res_it;
    ui_cache_res_t res;
    int rv = 0;
    
    /*查找所有重复的资源 */
    ui_data_src_group_srcs(&src_it, plugin_package_package_srcs(package));
    while((src = ui_data_src_it_next(&src_it))) {
        if (plugin_package_manip_src_is_base_provided(src, package)) {
            ui_data_src_group_add_src(remove_srcs, src);
        }
    }

    ui_cache_group_using_resources(&res_it, plugin_package_package_resources(package));
    while((res = ui_cache_res_it_next(&res_it))) {
        if (plugin_package_manip_res_is_base_provided(res, package)) {
            ui_cache_group_add_res(remove_resources, res);
        }
    }

    /*删除重复资源 */
    ui_data_src_group_srcs(&src_it, remove_srcs);
    while((src = ui_data_src_it_next(&src_it))) {
        if (ui_data_src_group_remove_src(plugin_package_package_srcs(package), src) != 0) rv = -1;
    }

    ui_cache_group_using_resources(&res_it, remove_resources);
    while((res = ui_cache_res_it_next(&res_it))) {
        ui_cache_group_remove_res(plugin_package_package_resources(package), res);
    }     
    
    /*释放临时缓存 */
    ui_data_src_group_free(remove_srcs);
    ui_cache_group_free(remove_resources);
    
    return 0;
}
