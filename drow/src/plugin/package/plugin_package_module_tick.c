#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/time_utils.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_mgr.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_group.h"
#include "plugin_package_module_i.h"
#include "plugin_package_package_i.h"
#include "plugin_package_depend_i.h"
#include "plugin_package_installer_i.h"
#include "plugin_package_queue_i.h"
#include "plugin_package_group_i.h"
#include "plugin_package_group_ref_i.h"
#include "plugin_package_region_i.h"
#include "plugin_package_load_task_i.h"

ptr_int_t plugin_package_module_tick(void * ctx, ptr_int_t arg, float delta_s) {
    plugin_package_module_t module = ctx;
    plugin_package_group_ref_t ref, next_ref;
    plugin_package_depend_t base_package;
    plugin_package_load_task_t load_task, next_load_task;

    /*处理需要下载的包 */
    if (module->m_to_download_packages) {
        for (ref = TAILQ_FIRST(&module->m_to_download_packages->m_packages);
             (module->m_downloading_package_limit == 0 || module->m_downloading_package_count < module->m_downloading_package_limit)
                 && ref != TAILQ_END(&module->m_to_download_packages->m_packages);
             ref = next_ref)
        {
            next_ref = TAILQ_NEXT(ref, m_next_for_group);
            
            switch(ref->m_package->m_state) {
            case plugin_package_package_empty:
                if (plugin_package_package_install(ref->m_package) != 0) {
                    CPE_ERROR(
                        module->m_em, "%s: tick: package %s install fail!",
                        plugin_package_module_name(module), plugin_package_package_name(ref->m_package));
                    plugin_package_group_remove_package(module->m_to_download_packages, ref->m_package);
                    module->m_total_download_complete_count++;
                    continue;
                }
                break;
            case plugin_package_package_downloading:
                break;
            case plugin_package_package_installed:
                plugin_package_group_remove_package(module->m_to_download_packages, ref->m_package);
                module->m_total_download_complete_count++;
                if (module->m_to_load_packages == NULL) {
                    module->m_to_load_packages = plugin_package_group_create(module, "to-download");
                    if (module->m_to_load_packages == NULL) {
                        CPE_ERROR(
                            module->m_em, "%s: tick: package %s from download to install: crate group fail!",
                            plugin_package_module_name(module), plugin_package_package_name(ref->m_package));
                        module->m_total_load_complete_count++;
                        continue;
                    }
                }
                if (plugin_package_group_add_package(module->m_to_load_packages, ref->m_package) != 0) {
                    CPE_ERROR(
                        module->m_em, "%s: tick: package %s add to install fail!",
                        plugin_package_module_name(module), plugin_package_package_name(ref->m_package));
                    module->m_total_load_complete_count++;
                    continue;
                }

                TAILQ_FOREACH(base_package, &ref->m_package->m_base_packages, m_next_for_extern) {
                    plugin_package_package_load_async(base_package->m_base_package, NULL);
                }
                break;
            case plugin_package_package_loading:
            case plugin_package_package_loaded:
            default:
                plugin_package_group_remove_package(module->m_to_download_packages, ref->m_package);
                module->m_total_download_complete_count++;
                break;
            }
        }

        if (TAILQ_EMPTY(&module->m_to_download_packages->m_packages)) {
            plugin_package_group_free(module->m_to_download_packages);
            module->m_to_download_packages = NULL;
        }
    }

    /*检查正在加载的包 */
    if (!TAILQ_EMPTY(&module->m_loading_packages)) {
        plugin_package_package_t package, next_package;
        int64_t start_time = cur_time_ms();
        for(package = TAILQ_FIRST(&module->m_loading_packages); package; package = next_package) {
            next_package = TAILQ_NEXT(package, m_next_for_module);
            
            plugin_package_package_load_async_tick(package, start_time);
            if (cur_time_ms() - start_time > module->m_process_tick_limit_ms) break;
        }
    }
    
    /*处理需要安装的包 */
    if (module->m_to_load_packages) {
        for (ref = TAILQ_FIRST(&module->m_to_load_packages->m_packages);
             (module->m_loading_package_limit == 0 || module->m_loading_package_count < module->m_loading_package_limit)
                 && ref != TAILQ_END(&module->m_to_load_packages->m_packages);
             ref = next_ref)
        {
            next_ref = TAILQ_NEXT(ref, m_next_for_group);
            
            switch(ref->m_package->m_state) {
            case plugin_package_package_empty:
            case plugin_package_package_downloading:
                CPE_ERROR(
                    module->m_em, "%s: tick: package %s is in state %d, error!",
                    plugin_package_module_name(module), plugin_package_package_name(ref->m_package), ref->m_package->m_state);
                plugin_package_group_remove_package(module->m_to_load_packages, ref->m_package);
                module->m_total_load_complete_count++;
                continue;
            case plugin_package_package_installed:
                if (plugin_package_package_load_async_start(ref->m_package) != 0) {
                    CPE_ERROR(
                        module->m_em, "%s: tick: package %s start fail!",
                        plugin_package_module_name(module), plugin_package_package_name(ref->m_package));
                    plugin_package_group_remove_package(module->m_to_load_packages, ref->m_package);
                    module->m_total_load_complete_count++;
                    continue;
                }
                break;
            case plugin_package_package_loading:
                break;
            case plugin_package_package_loaded:
                plugin_package_group_remove_package(module->m_to_load_packages, ref->m_package);
                module->m_total_load_complete_count++;
                break;
            default:
                CPE_ERROR(
                    module->m_em, "%s: tick: package %s unknown state %d!",
                    plugin_package_module_name(module), plugin_package_package_name(ref->m_package), ref->m_package->m_state);
                plugin_package_group_remove_package(module->m_to_load_packages, ref->m_package);
                module->m_total_load_complete_count++;
                continue;
            }
        }

        if (TAILQ_EMPTY(&module->m_to_load_packages->m_packages)) {
            plugin_package_group_free(module->m_to_load_packages);
            module->m_to_load_packages = NULL;
        }
    }

    /* printf("xxxxx: download %d/%d, load: %d/%d\n", */
    /*        module->m_total_download_complete_count , module->m_total_download_count, */
    /*        module->m_total_load_complete_count , module->m_total_load_count); */

    for(load_task = TAILQ_FIRST(&module->m_load_tasks); load_task; load_task = next_load_task) {
        next_load_task = TAILQ_NEXT(load_task, m_next_for_module);
        plugin_package_load_task_tick(load_task);
    }
    
    return 0;
}
