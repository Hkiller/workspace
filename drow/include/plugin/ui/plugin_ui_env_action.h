#ifndef DROW_PLUGIN_UI_ENV_ACTION_H
#define DROW_PLUGIN_UI_ENV_ACTION_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_env_action_it {
    plugin_ui_env_action_t (*next)(struct plugin_ui_env_action_it * it);
    char m_data[64];
};

plugin_ui_env_action_t
plugin_ui_env_action_create(
    plugin_ui_env_t env, 
    plugin_ui_event_t evt, 
    plugin_ui_event_fun_t fun, void * ctx);

void plugin_ui_env_action_free(plugin_ui_env_action_t action);

const char * plugin_ui_env_action_name_prefix(plugin_ui_env_action_t action);
int plugin_ui_env_action_set_name_prefix(plugin_ui_env_action_t action, const char * name_prefix);
    
uint8_t plugin_ui_env_action_data_capacity(plugin_ui_env_action_t action);
void * plugin_ui_env_action_data(plugin_ui_env_action_t action);

/*env_action_it*/    
#define plugin_ui_env_action_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif

