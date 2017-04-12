#ifndef PLUGIN_UI_PHASE_I_H
#define PLUGIN_UI_PHASE_I_H
#include "plugin/ui/plugin_ui_phase.h"
#include "plugin_ui_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_phase {
    plugin_ui_env_t m_env;
    TAILQ_ENTRY(plugin_ui_phase) m_next_for_env;
    
    char m_name[64];
    uint8_t m_fps;
    plugin_package_package_t m_package;
    plugin_ui_package_queue_using_list_t m_using_package_queues;
    plugin_ui_phase_use_page_list_t m_using_pages;
    plugin_ui_phase_use_popup_def_list_t m_using_popup_defs;
    struct cpe_hash_table m_states;
    plugin_ui_state_t m_init_state;
    plugin_ui_state_t m_init_call_state;
    plugin_ui_navigation_list_t m_navigations_as_back;    
};

int plugin_ui_phase_enter(plugin_ui_phase_t phase);
void plugin_ui_phase_leave(plugin_ui_phase_t phase);

#ifdef __cplusplus
}
#endif

#endif
