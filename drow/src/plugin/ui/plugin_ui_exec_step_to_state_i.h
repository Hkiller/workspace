#ifndef PLUGIN_UI_EXEC_STEP_TO_STATE_I_H
#define PLUGIN_UI_EXEC_STEP_TO_STATE_I_H
#include "plugin_ui_exec_step_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_exec_step_to_state {
    plugin_ui_state_t m_target_state;
    /*runtime*/
    plugin_ui_phase_node_t m_cur_phase;
    plugin_ui_state_t m_last_search_state;
    plugin_ui_state_t m_next_state;
    plugin_ui_navigation_t m_last_navigation;
};

#ifdef __cplusplus
}
#endif

#endif
