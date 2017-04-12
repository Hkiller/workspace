#ifndef DROW_PLUGIN_UI_PHASE_NODE_H
#define DROW_PLUGIN_UI_PHASE_NODE_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum plugin_ui_phase_node_state {
    plugin_ui_phase_node_state_prepare_loading,
    plugin_ui_phase_node_state_loading,
    plugin_ui_phase_node_state_prepare_back,
    plugin_ui_phase_node_state_back,
    plugin_ui_phase_node_state_suspend,
    plugin_ui_phase_node_state_prepare_resume,
    plugin_ui_phase_node_state_processing,
} plugin_ui_phase_node_state_t;

struct plugin_ui_phase_node_it {
    plugin_ui_phase_node_t (*next)(struct plugin_ui_phase_node_it * it);
    char m_data[64];
};
    
plugin_ui_aspect_t plugin_ui_phase_node_runtime_aspect(plugin_ui_phase_node_t phase_node);

plugin_ui_phase_node_t plugin_ui_phase_node_current(plugin_ui_env_t env);
plugin_ui_phase_node_t plugin_ui_phase_node_prev(plugin_ui_phase_node_t phase_node);

const char * plugin_ui_phase_node_name(plugin_ui_phase_node_t phase_node);
plugin_ui_phase_node_state_t plugin_ui_phase_node_state(plugin_ui_phase_node_t phase_node);
plugin_ui_phase_t plugin_ui_phase_node_loading_phase(plugin_ui_phase_node_t phase_node);
plugin_ui_phase_t plugin_ui_phase_node_process_phase(plugin_ui_phase_node_t phase_node);
plugin_ui_phase_t plugin_ui_phase_node_back_phase(plugin_ui_phase_node_t phase_node);
plugin_ui_phase_t plugin_ui_phase_node_current_phase(plugin_ui_phase_node_t phase_node);
const char * plugin_ui_phase_node_current_phase_name(plugin_ui_phase_node_t phase_node);
void plugin_ui_phase_node_state_nodes(plugin_ui_phase_node_t phase_node, plugin_ui_state_node_it_t state_node_it);

const char * plugin_ui_phase_node_op_str(plugin_ui_phase_node_t phase_node);
const char * plugin_ui_phase_node_state_str(plugin_ui_phase_node_t phase_node);
const char * plugin_ui_phase_node_state_to_str(plugin_ui_phase_node_state_t phase_node_state);

void plugin_ui_phase_node_print(write_stream_t s, plugin_ui_phase_node_t phase_node);
const char * plugin_ui_phase_node_dump(mem_buffer_t buffer, plugin_ui_phase_node_t phase_node);
    
#define plugin_ui_phase_node_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

