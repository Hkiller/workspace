#ifndef DROW_PLUGIN_UI_PACKAGE_QUEUE_MANAGED_H
#define DROW_PLUGIN_UI_PACKAGE_QUEUE_MANAGED_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_ui_package_queue_managed_t
plugin_ui_package_queue_managed_create(
    plugin_ui_env_t env, const char * name, plugin_package_queue_policy_t policy);

void plugin_ui_package_queue_managed_free(plugin_ui_package_queue_managed_t package_queue_managed);

plugin_ui_package_queue_managed_t
plugin_ui_package_queue_managed_find(plugin_ui_env_t env, const char * name);

#ifdef __cplusplus
}
#endif

#endif

