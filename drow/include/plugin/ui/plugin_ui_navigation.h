#ifndef DROW_PLUGIN_UI_NAVIGATION_H
#define DROW_PLUGIN_UI_NAVIGATION_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_navigation_it {
    plugin_ui_navigation_t (*next)(struct plugin_ui_navigation_it * it);
    char m_data[64];
};
    
const char * plugin_ui_navigation_trigger_control(plugin_ui_navigation_t navigation);
int plugin_ui_navigation_set_trigger_control(plugin_ui_navigation_t navigation, const char * control_path);

const char * plugin_ui_navigation_condition(plugin_ui_navigation_t navigation);
int plugin_ui_navigation_set_condition(plugin_ui_navigation_t navigation, const char * condition);

void plugin_ui_navigation_free(plugin_ui_navigation_t navigation);

plugin_ui_state_t plugin_ui_navigation_from(plugin_ui_navigation_t navigation);
plugin_ui_navigation_category_t plugin_ui_navigation_category(plugin_ui_navigation_t navigation);
plugin_ui_renter_policy_t plugin_ui_navigation_renter_policy(plugin_ui_navigation_t navigation);

float plugin_ui_navigation_weight(plugin_ui_navigation_t navigation);
void plugin_ui_navigation_set_weight(plugin_ui_navigation_t navigation, float weight);

void * plugin_ui_navigation_data(plugin_ui_navigation_t navigation);
int plugin_ui_navigation_execute(plugin_ui_navigation_t navigation, plugin_ui_state_node_t state_node, dr_data_t data);

/*state*/
plugin_ui_navigation_t
plugin_ui_navigation_state_create(
    plugin_ui_state_t from, plugin_ui_state_t to,
    plugin_ui_navigation_state_op_t op, plugin_ui_renter_policy_t renter_policy);

plugin_ui_state_t plugin_ui_navigation_state_to(plugin_ui_navigation_t navigation);
void plugin_ui_navigation_state_set_to(plugin_ui_navigation_t navigation, plugin_ui_state_t to);
    
plugin_ui_navigation_state_op_t plugin_ui_navigation_state_op(plugin_ui_navigation_t navigation);
const char * plugin_ui_navigation_state_op_str(plugin_ui_navigation_t navigation);
    
plugin_ui_navigation_state_base_policy_t plugin_ui_navigation_state_base_policy(plugin_ui_navigation_t navigation);
void plugin_ui_navigation_state_set_base_policy(plugin_ui_navigation_t navigation, plugin_ui_navigation_state_base_policy_t  base_policy);

uint8_t plugin_ui_navigation_state_suspend(plugin_ui_navigation_t navigation);
void plugin_ui_navigation_state_set_suspend(plugin_ui_navigation_t navigation, uint8_t suspend);
    
plugin_ui_state_t plugin_ui_navigation_state_loading(plugin_ui_navigation_t navigation);
void plugin_ui_navigation_state_set_loading(plugin_ui_navigation_t navigation, plugin_ui_state_t loading_state);

plugin_ui_state_t plugin_ui_navigation_state_back(plugin_ui_navigation_t navigation);    
void plugin_ui_navigation_state_set_back(plugin_ui_navigation_t navigation, plugin_ui_state_t back_state);
    
const char * plugin_ui_navigation_state_op_to_str(plugin_ui_navigation_state_op_t op);

/*phase*/
plugin_ui_navigation_t
plugin_ui_navigation_phase_create(
    plugin_ui_state_t from, plugin_ui_phase_t to,
    plugin_ui_navigation_phase_op_t op, plugin_ui_renter_policy_t renter_policy);

plugin_ui_navigation_phase_op_t plugin_ui_navigation_phase_op(plugin_ui_navigation_t navigation);
const char * plugin_ui_navigation_phase_op_str(plugin_ui_navigation_t navigation);
    
plugin_ui_phase_t plugin_ui_navigation_phase_to(plugin_ui_navigation_t navigation);
void plugin_ui_navigation_phase_set_to(plugin_ui_navigation_t navigation, plugin_ui_phase_t to);
    
plugin_ui_phase_t plugin_ui_navigation_phase_loading(plugin_ui_navigation_t navigation);
uint8_t plugin_ui_navigation_phase_loading_auto_complete(plugin_ui_navigation_t navigation);
void plugin_ui_navigation_phase_set_loading(plugin_ui_navigation_t navigation, plugin_ui_phase_t loading_phase, uint8_t auto_complete);

plugin_ui_phase_t plugin_ui_navigation_phase_back(plugin_ui_navigation_t navigation);    
uint8_t plugin_ui_navigation_phase_back_auto_complete(plugin_ui_navigation_t navigation);
void plugin_ui_navigation_phase_set_back(plugin_ui_navigation_t navigation, plugin_ui_phase_t back_phase, uint8_t auto_complete);
    
const char * plugin_ui_navigation_phase_op_to_str(plugin_ui_navigation_phase_op_t op);
    
/*it*/
#define plugin_ui_navigation_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

