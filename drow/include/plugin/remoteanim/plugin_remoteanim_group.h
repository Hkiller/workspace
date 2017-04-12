#ifndef UI_PLUGIN_REMOTEANIM_GROUP_H
#define UI_PLUGIN_REMOTEANIM_GROUP_H
#include "plugin_remoteanim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_remoteanim_group_t
plugin_remoteanim_group_create(plugin_remoteanim_module_t module, const char * name, ui_vector_2_t capacity);
void plugin_remoteanim_group_free(plugin_remoteanim_group_t group);

plugin_remoteanim_group_t
plugin_remoteanim_group_find(plugin_remoteanim_module_t module, const char * name);

ui_vector_2_t plugin_remoteanim_group_capacity(plugin_remoteanim_group_t group);
net_trans_group_t plugin_remoteanim_group_trans_group(plugin_remoteanim_group_t group);
    
#ifdef __cplusplus
}
#endif

#endif

