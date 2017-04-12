#ifndef DROW_PLUGIN_UI_CONTROL_META_H
#define DROW_PLUGIN_UI_CONTROL_META_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*plugin_ui_control_init_fun_t)(plugin_ui_control_t control);
typedef void (*plugin_ui_control_fini_fun_t)(plugin_ui_control_t control);
typedef int (*plugin_ui_control_load_fun_t)(plugin_ui_control_t control);
typedef void (*plugin_ui_control_layout_fun_t)(plugin_ui_control_t control, ui_vector_2_t client_sz);
typedef void (*plugin_ui_control_update_fun_t)(plugin_ui_control_t control, float delta);

typedef void (*plugin_ui_control_event_fun_t)(plugin_ui_control_t control);
typedef void (*plugin_ui_control_event_2_fun_t)(plugin_ui_control_t control, plugin_ui_control_t other);
    
plugin_ui_control_meta_t
plugin_ui_control_meta_create(
    plugin_ui_module_t module,
    uint8_t type,
    uint32_t data_capacity,
    plugin_ui_control_init_fun_t init_fun,
    plugin_ui_control_fini_fun_t fini_fun,
    plugin_ui_control_load_fun_t load_fun,
    plugin_ui_control_update_fun_t update_fun);

void plugin_ui_module_unregister_control(plugin_ui_module_t module, uint8_t type);

plugin_ui_control_meta_t
plugin_ui_control_meta_find(plugin_ui_module_t module, uint8_t type);

void plugin_ui_control_meta_set_layout(plugin_ui_control_meta_t meta, plugin_ui_control_layout_fun_t layout);
    
void plugin_ui_control_meta_set_on_active(plugin_ui_control_meta_t meta, plugin_ui_control_event_fun_t on_active);
void plugin_ui_control_meta_set_on_deactive(plugin_ui_control_meta_t meta, plugin_ui_control_event_fun_t on_deactive);
void plugin_ui_control_meta_set_on_self_loaded(plugin_ui_control_meta_t meta, plugin_ui_control_event_fun_t on_self_loaded);

void plugin_ui_control_meta_set_event_fun(
    plugin_ui_control_meta_t meta, plugin_ui_event_t evt, plugin_ui_event_scope_t scope, plugin_ui_event_fun_t fun);
    
#ifdef __cplusplus
}
#endif

#endif

