#ifndef UI_PLUGIN_PACKAGE_GROUP_H
#define UI_PLUGIN_PACKAGE_GROUP_H
#include "plugin_package_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_package_group_t plugin_package_group_create(plugin_package_module_t module, const char * name);
void plugin_package_group_free(plugin_package_group_t group);

plugin_package_module_t plugin_package_group_module(plugin_package_group_t group);

const char * plugin_package_group_name(plugin_package_group_t group);
void plugin_package_group_set_name(plugin_package_group_t group, const char * name);

plugin_package_package_using_state_t plugin_package_group_using_state(plugin_package_group_t group);
void plugin_package_group_set_using_state(plugin_package_group_t group, plugin_package_package_using_state_t using_state);
    
uint32_t plugin_package_group_package_count(plugin_package_group_t group);
void plugin_package_group_remove_package(plugin_package_group_t group, plugin_package_package_t package);
int plugin_package_group_add_package(plugin_package_group_t group, plugin_package_package_t package);
int plugin_package_group_add_packages(plugin_package_group_t group, plugin_package_group_t from_group);
int plugin_package_group_add_package_r(plugin_package_group_t group, plugin_package_package_t package);
int plugin_package_group_add_packages_r(plugin_package_group_t group, plugin_package_group_t from_group);

int plugin_package_group_add_base_packages_r(plugin_package_group_t group, plugin_package_package_t package);

int plugin_package_group_load_sync(plugin_package_group_t group);
int plugin_package_group_load_async(plugin_package_group_t group, plugin_package_load_task_t task);

void plugin_package_group_clear(plugin_package_group_t group);

int plugin_package_group_expand_base_packages(plugin_package_group_t group);
    
void plugin_package_group_packages(plugin_package_package_it_t it, plugin_package_group_t group);
    

#ifdef __cplusplus
}
#endif

#endif

