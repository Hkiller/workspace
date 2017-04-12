#ifndef DROW_PLUGIN_UI_STATE_NODE_H
#define DROW_PLUGIN_UI_STATE_NODE_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_state_node_it {
    plugin_ui_state_node_t (*next)(struct plugin_ui_state_node_it * it);
    char m_data[64];
};
    
typedef enum plugin_ui_state_node_state {
    plugin_ui_state_node_state_prepare_loading,
    plugin_ui_state_node_state_loading,
    plugin_ui_state_node_state_prepare_back,
    plugin_ui_state_node_state_back,
    plugin_ui_state_node_state_suspend,
    plugin_ui_state_node_state_prepare_resume,
    plugin_ui_state_node_state_processing,
} plugin_ui_state_node_state_t;
    
plugin_ui_state_node_t plugin_ui_state_node_current(plugin_ui_phase_node_t phase_node);
plugin_ui_state_node_t plugin_ui_state_node_prev(plugin_ui_state_node_t state_node);
plugin_ui_state_node_t plugin_ui_state_node_find_by_process(plugin_ui_phase_node_t phase_node, const char * process_state_name);
plugin_ui_state_node_t plugin_ui_state_node_find_by_current(plugin_ui_phase_node_t phase_node, const char * state_name);
plugin_ui_state_node_t plugin_ui_state_node_find_by_level(plugin_ui_phase_node_t phase_node, uint8_t level);

uint8_t plugin_ui_state_node_level(plugin_ui_state_node_t state_node);

const char * plugin_ui_state_node_name(plugin_ui_state_node_t state_node);
plugin_ui_state_node_state_t plugin_ui_state_node_state(plugin_ui_state_node_t state_node);
plugin_ui_state_t plugin_ui_state_node_loading_state(plugin_ui_state_node_t state_node);
plugin_ui_state_t plugin_ui_state_node_process_state(plugin_ui_state_node_t state_node);
plugin_ui_state_t plugin_ui_state_node_back_state(plugin_ui_state_node_t state_node);
plugin_ui_state_t plugin_ui_state_node_current_state(plugin_ui_state_node_t state_node);
dr_data_t plugin_ui_state_node_data(plugin_ui_state_node_t state_node);

plugin_ui_state_node_t
plugin_ui_state_node_call(
    plugin_ui_state_node_t from_node, const char * state_name, const char * loading_state_name, const char * back_state_name,
    plugin_ui_renter_policy_t renter_policy, uint8_t suspend_old, dr_data_t data);

int plugin_ui_state_node_switch(
    plugin_ui_state_node_t from_node, const char * state_name, const char * loading_state_name, const char * back_state_name,
    dr_data_t data);
int plugin_ui_state_node_reset(plugin_ui_phase_node_t phase_node);
void plugin_ui_state_node_back(plugin_ui_state_node_t state_node);
void plugin_ui_state_node_remove(plugin_ui_state_node_t state_node);

const char * plugin_ui_state_node_op_str(plugin_ui_state_node_t state_node);
const char * plugin_ui_state_node_state_str(plugin_ui_state_node_t state_node);
const char * plugin_ui_state_node_state_to_str(plugin_ui_state_node_state_t state_node_state);

#define plugin_ui_state_node_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

