#ifndef UI_PLUGIN_PACKAGE_QUEUE_H
#define UI_PLUGIN_PACKAGE_QUEUE_H
#include "plugin_package_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_package_queue_t
plugin_package_queue_create(
    plugin_package_module_t module, const char * name, plugin_package_queue_policy_t policy);
void plugin_package_queue_free(plugin_package_queue_t queue);

plugin_package_queue_t
plugin_package_queue_find(plugin_package_module_t module, const char * name);

const char * plugin_package_queue_name(plugin_package_queue_t queue);
uint32_t plugin_package_queue_limit(plugin_package_queue_t queue);
void plugin_package_queue_set_limit(plugin_package_queue_t queue, uint32_t limit);

uint32_t plugin_package_queue_package_count(plugin_package_queue_t queue);
void plugin_package_queue_remove_package(plugin_package_queue_t queue, plugin_package_package_t package);
int plugin_package_queue_add_package(plugin_package_queue_t queue, plugin_package_package_t package);
int plugin_package_queue_add_packages(plugin_package_queue_t queue, plugin_package_group_t group);    

void plugin_package_queue_clear(plugin_package_queue_t queue);

int plugin_package_queue_load_all_async(plugin_package_queue_t queue, plugin_package_load_task_t task);
    
void plugin_package_queues(plugin_package_queue_it_t queue_it, plugin_package_module_t module);

/*it*/
struct plugin_package_queue_it {
    plugin_package_queue_t (*next)(plugin_package_queue_it_t it);
    char m_data[64];
};

#define plugin_package_queue_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

