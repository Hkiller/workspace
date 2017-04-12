#ifndef DROW_PLUGIN_UI_PACKAGE_QUEUE_USING_H
#define DROW_PLUGIN_UI_PACKAGE_QUEUE_USING_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_ui_package_queue_using_t
plugin_ui_package_queue_using_create(plugin_ui_package_queue_managed_t queue, plugin_ui_phase_t phase);
void plugin_ui_package_queue_using_free(plugin_ui_package_queue_using_t package_queue_using);

plugin_ui_package_queue_using_t
plugin_ui_package_queue_using_find(plugin_ui_package_queue_managed_t queue, plugin_ui_phase_t phase);
    
uint32_t plugin_ui_package_queue_using_limit(plugin_ui_package_queue_using_t using);
void plugin_ui_package_queue_using_set_limit(plugin_ui_package_queue_using_t using, uint32_t limit);
    
#ifdef __cplusplus
}
#endif

#endif

