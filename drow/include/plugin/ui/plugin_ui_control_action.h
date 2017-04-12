#ifndef DROW_PLUGIN_UI_CONTROL_ACTION_H
#define DROW_PLUGIN_UI_CONTROL_ACTION_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_control_action_it {
    plugin_ui_control_action_t (*next)(struct plugin_ui_control_action_it * it);
    char m_data[64];
};

plugin_ui_control_action_t
plugin_ui_control_action_create(
    plugin_ui_control_t control, 
    plugin_ui_event_t evt, plugin_ui_event_scope_t scope,
    plugin_ui_event_fun_t fun, void * ctx);

uint8_t plugin_ui_control_have_action(
        plugin_ui_control_t control, 
        plugin_ui_event_t evt, plugin_ui_event_scope_t scope);
    
void plugin_ui_control_action_free(plugin_ui_control_action_t action);

void plugin_ui_control_action_remove_in_page_by_func(plugin_ui_page_t page, plugin_ui_event_fun_t fun);
    
const char * plugin_ui_control_action_name_prefix(plugin_ui_control_action_t action);
int plugin_ui_control_action_set_name_prefix(plugin_ui_control_action_t action, const char * name_prefix);
    
uint8_t plugin_ui_control_action_data_capacity(plugin_ui_control_action_t action);
void * plugin_ui_control_action_data(plugin_ui_control_action_t action);

/*control_action_it*/    
#define plugin_ui_control_action_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif

