#ifndef PLUGIN_UI_CONTROL_ACTION_I_H
#define PLUGIN_UI_CONTROL_ACTION_I_H
#include "plugin/ui/plugin_ui_control_action.h"
#include "plugin_ui_control_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_control_action {
    plugin_ui_control_t m_control;
    TAILQ_ENTRY(plugin_ui_control_action) m_next;
    TAILQ_ENTRY(plugin_ui_control_action) m_next_for_page;
    plugin_ui_aspect_ref_list_t m_aspects;
    
    char * m_name_prefix; 
    plugin_ui_event_t m_event;
    plugin_ui_event_scope_t m_scope;
    plugin_ui_event_fun_t m_fun;
    void * m_ctx;
    char m_data[PLUGIN_UI_CONTROL_ACTION_DATA_CAPACITY];
};

void plugin_ui_control_action_real_free(plugin_ui_control_action_t action);

#ifdef __cplusplus
}
#endif

#endif
