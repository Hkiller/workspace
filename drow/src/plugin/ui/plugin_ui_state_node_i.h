#ifndef PLUGIN_UI_STATE_NODE_I_H
#define PLUGIN_UI_STATE_NODE_I_H
#include "plugin/ui/plugin_ui_state_node.h"
#include "plugin_ui_state_i.h"
#include "plugin_ui_phase_node_i.h"

#ifdef __cplusplus
extern "C" {
#endif

enum plugin_ui_state_node_op {
    plugin_ui_state_node_op_none,
    plugin_ui_state_node_op_init,
    plugin_ui_state_node_op_remove,
    plugin_ui_state_node_op_resume,
    plugin_ui_state_node_op_back,
    plugin_ui_state_node_op_switch,
};

struct plugin_ui_state_node_data {
    uint8_t m_suspend_old;
    plugin_ui_state_t m_process_state;
    plugin_ui_state_t m_loading_state;    
    plugin_ui_state_t m_back_state;
    plugin_package_group_t m_inout_packages;
    plugin_package_group_t m_runtime_packages;
    struct dr_data m_data;
    char m_data_inline_buf[64];
};

struct plugin_ui_state_node {
    plugin_ui_phase_node_t m_phase_node;    
    TAILQ_ENTRY(plugin_ui_state_node) m_next;
    uint32_t m_package_load_task;
    plugin_ui_navigation_t m_from_navigation;
    plugin_ui_state_node_page_list_t m_pages;
    uint8_t m_level;
    uint8_t m_is_active;
    enum plugin_ui_state_node_state m_state;
    enum plugin_ui_state_node_op m_op;
    struct plugin_ui_state_node_data m_curent;
    struct plugin_ui_state_node_data m_replace;    
};

plugin_ui_state_node_t
plugin_ui_state_node_create(
    plugin_ui_phase_node_t phase_node,
    plugin_ui_state_t process_state,
    plugin_ui_state_t loading_state,
    plugin_ui_state_t back_state,
    uint8_t suspend_old, dr_data_t data);
    
void plugin_ui_state_node_free(plugin_ui_state_node_t state_node);
void plugin_ui_state_node_real_free(plugin_ui_state_node_t state_node);    

void plugin_ui_state_node_suspend(plugin_ui_state_node_t state_node);
    
uint8_t plugin_ui_state_node_is_active(plugin_ui_state_node_t state_node);
int plugin_ui_state_node_active(plugin_ui_state_node_t state_node, plugin_ui_state_node_state_t next_state);

void plugin_ui_state_node_clear_replace(plugin_ui_state_node_t node);
void plugin_ui_state_node_replace(plugin_ui_state_node_t node, uint8_t force_replace_data);
    
int plugin_ui_state_node_switch_i(
    plugin_ui_state_node_t from_node, plugin_ui_state_t state, plugin_ui_state_t loading_state, plugin_ui_state_t back_state, dr_data_t data);

plugin_ui_state_node_t
plugin_ui_state_node_call_i(
    plugin_ui_state_node_t from_node, plugin_ui_state_t state, plugin_ui_state_t loading_state, plugin_ui_state_t back_state, 
    plugin_ui_renter_policy_t renter_policy, uint8_t suspend_old, dr_data_t data);
    
int plugin_ui_state_node_data_init(
    plugin_ui_env_t env, plugin_ui_state_node_data_t node_data,
    plugin_ui_state_t process_state, plugin_ui_state_t loading_state, plugin_ui_state_t back_state,
    uint8_t suspend_old, dr_data_t data);
    
void plugin_ui_state_node_data_fini(
    plugin_ui_env_t env, plugin_ui_state_node_data_t node_data);

const char * plugin_ui_state_node_op_to_str(enum plugin_ui_state_node_op op);

void plugin_ui_state_node_stop_package_load_task(plugin_ui_state_node_t state_node);
uint8_t plugin_ui_state_node_check_package_load_task_runing(plugin_ui_state_node_t state_node);
plugin_package_load_task_t plugin_ui_state_node_create_package_load_task(plugin_ui_state_node_t state_node);

plugin_package_group_t plugin_ui_state_node_inout_packages_check_create(plugin_ui_state_node_t state_node);
plugin_package_group_t plugin_ui_state_node_runtime_packages_check_create(plugin_ui_state_node_t state_node);
    
#ifdef __cplusplus
}
#endif

#endif
