#ifndef PLUGIN_UI_PHASE_NODE_I_H
#define PLUGIN_UI_PHASE_NODE_I_H
#include "plugin/ui/plugin_ui_phase_node.h"
#include "plugin_ui_phase_i.h"

#ifdef __cplusplus
extern "C" {
#endif

enum plugin_ui_phase_node_op {
    plugin_ui_phase_node_op_none,
    plugin_ui_phase_node_op_init,
    plugin_ui_phase_node_op_enter,
    plugin_ui_phase_node_op_suspend,
    plugin_ui_phase_node_op_resume,
    plugin_ui_phase_node_op_remove,
    plugin_ui_phase_node_op_back,    
};
    
struct plugin_ui_phase_node {
    plugin_ui_env_t m_env;
    TAILQ_ENTRY(plugin_ui_phase_node) m_next;
    plugin_package_group_t m_inout_packages;
    plugin_package_group_t m_runtime_packages;
    uint32_t m_package_load_task;
    uint8_t m_is_processed;
    enum plugin_ui_phase_node_state m_state;
    enum plugin_ui_phase_node_op m_op;
    plugin_ui_phase_t m_process_phase;
    plugin_ui_phase_t m_loading_phase;
    uint8_t m_loading_auto_complete;
    plugin_ui_phase_t m_back_phase;
    uint8_t m_back_auto_complete;
    plugin_ui_state_node_list_t m_state_stack;
    plugin_ui_aspect_t m_runtime_aspect;
    struct dr_data m_data;
    char m_data_inline_buf[64];
};

plugin_ui_phase_node_t
plugin_ui_phase_node_create(
    plugin_ui_env_t env,
    plugin_ui_phase_t process_phase,
    plugin_ui_phase_t loading_phase, uint8_t loading_auto_complete,
    plugin_ui_phase_t back_phase, uint8_t back_auto_complete,
    dr_data_t data);
    
void plugin_ui_phase_node_free(plugin_ui_phase_node_t phase_node);
void plugin_ui_phase_node_real_free(plugin_ui_phase_node_t phase_node);    
    
void plugin_ui_phase_node_clear_stack(plugin_ui_phase_node_t phase_node);
void plugin_ui_phase_node_suspend_stack(plugin_ui_phase_node_t phase_node);

void plugin_ui_phase_node_stop_package_load_task(plugin_ui_phase_node_t phase_node);
uint8_t plugin_ui_phase_node_check_package_load_task_runing(plugin_ui_phase_node_t phase_node);
plugin_package_load_task_t plugin_ui_phase_node_create_package_load_task(plugin_ui_phase_node_t phase_node);
    
const char * plugin_ui_phase_node_op_to_str(enum plugin_ui_phase_node_op op);

#ifdef __cplusplus
}
#endif

#endif
