#ifndef DROW_PLUGIN_UI_CONTROL_BINDING_H
#define DROW_PLUGIN_UI_CONTROL_BINDING_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_ui_control_binding_t
plugin_ui_control_binding_create(plugin_ui_control_t control, const char * attr_name);
void plugin_ui_control_binding_free(plugin_ui_control_binding_t binding);

const char * plugin_ui_control_binding_attr_name(plugin_ui_control_binding_t bining);

const char * plugin_ui_control_binding_finction(plugin_ui_control_binding_t bining);
int plugin_ui_control_binding_set_function(plugin_ui_control_binding_t bining, const char * function);

uint8_t plugin_ui_control_binding_need_process(plugin_ui_control_binding_t binding);
void plugin_ui_control_binding_set_need_process(plugin_ui_control_binding_t binding, uint8_t need_process);
    
int plugin_ui_control_binding_apply(plugin_ui_control_binding_t binding);

#ifdef __cplusplus
}
#endif

#endif

