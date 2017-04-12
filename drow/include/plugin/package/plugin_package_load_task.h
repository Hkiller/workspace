#ifndef UI_PLUGIN_PACKAGE_LOAD_TASK_H
#define UI_PLUGIN_PACKAGE_LOAD_TASK_H
#include "plugin_package_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*plugin_package_load_task_progress_fun_t)(
    void * ctx, plugin_package_load_task_t task, plugin_package_package_t package, float progress);

plugin_package_load_task_t
plugin_package_load_task_create(
    plugin_package_module_t module, void * ctx, plugin_package_load_task_progress_fun_t fun, size_t carry_data_size);
void plugin_package_load_task_free(plugin_package_load_task_t task);

void plugin_package_load_task_free_by_ctx(plugin_package_module_t module, void * ctx);

int plugin_package_load_task_add_package(plugin_package_load_task_t task, plugin_package_package_t package);
int plugin_package_load_task_add_package_r(plugin_package_load_task_t task, plugin_package_package_t package);

uint32_t plugin_package_load_task_id(plugin_package_load_task_t task);
plugin_package_load_task_t plugin_package_load_task_find_by_id(plugin_package_module_t module, uint32_t id);    

void * plugin_package_load_task_carry_data(plugin_package_load_task_t task, size_t size);

uint32_t plugin_package_load_task_total_download_count(plugin_package_load_task_t task);
uint32_t plugin_package_load_task_total_package_count(plugin_package_load_task_t task);
void plugin_package_load_task_packages(plugin_package_load_task_t task, plugin_package_package_it_t it);
    
#ifdef __cplusplus
}
#endif

#endif

