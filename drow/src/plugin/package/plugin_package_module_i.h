#ifndef UI_PLUGIN_PARTICLE_MODULE_I_H
#define UI_PLUGIN_PARTICLE_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "plugin/package/plugin_package_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_package_group_ref * plugin_package_group_ref_t;
    
typedef TAILQ_HEAD(plugin_package_package_list, plugin_package_package) plugin_package_package_list_t;
typedef TAILQ_HEAD(plugin_package_language_list, plugin_package_language) plugin_package_language_list_t;
typedef TAILQ_HEAD(plugin_package_load_task_list, plugin_package_load_task) plugin_package_load_task_list_t;
typedef TAILQ_HEAD(plugin_package_group_list, plugin_package_group) plugin_package_group_list_t;
typedef TAILQ_HEAD(plugin_package_group_ref_list, plugin_package_group_ref) plugin_package_group_ref_list_t;
typedef TAILQ_HEAD(plugin_package_region_list, plugin_package_region) plugin_package_region_list_t;
typedef TAILQ_HEAD(plugin_package_depend_list, plugin_package_depend) plugin_package_depend_list_t;
    
struct plugin_package_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;
    uint8_t m_is_control_res;

    ui_data_mgr_t m_data_mgr;
    ui_cache_manager_t m_cache_mgr;
    char * m_repo_path;
    plugin_package_installer_t m_installer;

    struct cpe_hash_table m_packages;
    uint16_t m_downloading_package_count;
    plugin_package_package_list_t m_downloading_packages;
    uint16_t m_loading_package_count;
    plugin_package_package_list_t m_loading_packages;
    uint16_t m_loaded_package_count;
    plugin_package_package_list_t m_loaded_packages;

    struct cpe_hash_table m_queues;

    plugin_package_region_list_t m_regions;    
    plugin_package_group_list_t m_groups;
    uint16_t m_max_load_task_id;
    plugin_package_load_task_list_t m_load_tasks;

    plugin_package_load_task_list_t m_free_load_tasks;
    plugin_package_group_list_t m_free_groups;
    plugin_package_group_ref_list_t m_free_group_refs;

    /*异步加载相关数据 */
    uint16_t m_downloading_package_limit;
    uint16_t m_loading_package_limit;
    uint16_t m_process_tick_limit_ms;    
    plugin_package_group_t m_to_load_packages;
    plugin_package_group_t m_to_download_packages;

    /*统计 */
    uint16_t m_total_load_complete_count;
    uint16_t m_total_load_count;
    uint16_t m_total_download_complete_count;
    uint16_t m_total_download_count;
};

ptr_int_t plugin_package_module_tick(void * ctx, ptr_int_t arg, float delta_s);
    
#ifdef __cplusplus
}
#endif

#endif
