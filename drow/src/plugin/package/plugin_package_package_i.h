#ifndef UI_PLUGIN_PACKAGE_PACKAGE_I_H
#define UI_PLUGIN_PACKAGE_PACKAGE_I_H
#include "plugin/package/plugin_package_package.h"
#include "plugin_package_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_package_package {
    plugin_package_module_t m_module;
    cpe_hash_entry m_hh;
    TAILQ_ENTRY(plugin_package_package) m_next_for_module;
    plugin_package_group_ref_list_t m_groups;
    plugin_package_depend_list_t m_base_packages;
    plugin_package_depend_list_t m_extern_packages;
    const char * m_name;
    char * m_path;
    plugin_package_package_state_t m_state;
    ui_cache_group_t m_resources;
    ui_data_src_group_t m_srcs;
    plugin_package_language_list_t m_languages;
    plugin_package_language_t m_active_language;

    ui_cache_group_t m_loading_res_group;
    ui_data_src_group_t m_loading_src_group;
    uint32_t m_total_size;
    float m_progress;
};

void plugin_package_package_free_all(plugin_package_module_t module);

uint32_t plugin_package_package_hash(plugin_package_package_t package);
int plugin_package_package_eq(plugin_package_package_t l, plugin_package_package_t r);

int plugin_package_package_load_async_start(plugin_package_package_t package);    
int plugin_package_package_load_async_tick(plugin_package_package_t package, int64_t start_time);

void plugin_package_package_set_state(plugin_package_package_t package, plugin_package_package_state_t state);
    
#ifdef __cplusplus
}
#endif

#endif
