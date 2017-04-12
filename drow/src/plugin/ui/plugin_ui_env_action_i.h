#ifndef PLUGIN_UI_ENV_ACTION_I_H
#define PLUGIN_UI_ENV_ACTION_I_H
#include "plugin/ui/plugin_ui_env_action.h"
#include "plugin_ui_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_env_action {
    plugin_ui_env_t m_env;
    TAILQ_ENTRY(plugin_ui_env_action) m_next_for_env;
    plugin_ui_aspect_ref_list_t m_aspects;
    uint8_t m_is_processing;
    uint8_t m_is_free;
    
    char * m_name_prefix; 
    plugin_ui_event_t m_event;
    plugin_ui_event_fun_t m_fun;
    void * m_ctx;
    char m_data[PLUGIN_UI_CONTROL_ACTION_DATA_CAPACITY];
};

void plugin_ui_env_action_real_free(plugin_ui_env_action_t action);

#ifdef __cplusplus
}
#endif

#endif
