#ifndef PLUGIN_UI_NAVIGATION_I_H
#define PLUGIN_UI_NAVIGATION_I_H
#include "plugin/ui/plugin_ui_navigation.h"
#include "plugin_ui_state_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_navigation {
    plugin_ui_env_t m_env;
    plugin_ui_navigation_category_t m_category;
    char * m_trigger_control;
    char * m_condition;
    float m_weight;
    plugin_ui_renter_policy_t m_renter_policy;
    plugin_ui_state_t m_from_state;
    TAILQ_ENTRY(plugin_ui_navigation) m_next_for_from;
    plugin_ui_state_t m_to_state;
    TAILQ_ENTRY(plugin_ui_navigation) m_next_for_to;
    plugin_ui_state_t m_loading_state;
    TAILQ_ENTRY(plugin_ui_navigation) m_next_for_loading;
    plugin_ui_state_t m_back_state;    
    TAILQ_ENTRY(plugin_ui_navigation) m_next_for_back;
    union {
        struct {
            plugin_ui_navigation_state_op_t m_op;
            uint8_t m_suspend;
            plugin_ui_navigation_state_base_policy_t m_base_policy;
        } m_state;
        struct {
            plugin_ui_navigation_phase_op_t m_op;
            uint8_t m_loading_auto_complete;
            uint8_t m_back_auto_complete;
        } m_phase;
    };
};

/*state*/
int plugin_ui_navigation_state_execute(plugin_ui_navigation_t navigation, plugin_ui_state_node_t state_node, dr_data_t data);

/*phase*/
int plugin_ui_navigation_phase_execute(plugin_ui_navigation_t navigation, plugin_ui_phase_node_t phase_node, dr_data_t data);
    
#ifdef __cplusplus
}
#endif

#endif
