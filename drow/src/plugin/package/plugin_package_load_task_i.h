#ifndef PLUGIN_PACKAGE_LOAD_TASK_I_H
#define PLUGIN_PACKAGE_LOAD_TASK_I_H
#include "plugin/package/plugin_package_load_task.h"
#include "plugin_package_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_package_load_task {
    plugin_package_module_t m_module;
    TAILQ_ENTRY(plugin_package_load_task) m_next_for_module;
    uint32_t m_id;
    plugin_package_group_t m_group;
    void * m_ctx;
    plugin_package_load_task_progress_fun_t m_process_fun;
    char m_carry_data[64];
    uint8_t m_carry_data_size;
    uint32_t m_total_download_count;
    uint32_t m_total_package_count;
    uint32_t m_left_package_count;
    float m_process;
};

void plugin_package_load_task_real_free(plugin_package_load_task_t task);

void plugin_package_load_task_tick(plugin_package_load_task_t task);
    
#ifdef __cplusplus
}
#endif

#endif
