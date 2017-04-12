#ifndef DROW_PLUGIN_UI_CONTROL_ATTR_META_H
#define DROW_PLUGIN_UI_CONTROL_ATTR_META_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*plugin_ui_control_attr_set_fun_t)(plugin_ui_control_t control, dr_value_t data);
typedef int (*plugin_ui_control_attr_get_fun_t)(plugin_ui_control_t control, dr_value_t data);
    
plugin_ui_control_attr_meta_t
plugin_ui_control_attr_meta_create(
    plugin_ui_control_meta_t control_meta, const char * attr_name,
    plugin_ui_control_attr_set_fun_t setter,
    plugin_ui_control_attr_get_fun_t getter);
    
void plugin_ui_control_attr_meta_free(plugin_ui_control_attr_meta_t attr_meta);

plugin_ui_control_attr_meta_t
plugin_ui_control_attr_meta_find(plugin_ui_control_meta_t control_meta, const char * attr_name);
    
#ifdef __cplusplus
}
#endif

#endif

