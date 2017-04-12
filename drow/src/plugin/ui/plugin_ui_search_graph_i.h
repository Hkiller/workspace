#ifndef PLUGIN_UI_SEARCH_GRAPH_I_H
#define PLUGIN_UI_SEARCH_GRAPH_I_H
#include "plugin/ui/plugin_ui_search.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_search_graph_node {
    plugin_ui_state_t m_state;
    plugin_ui_state_node_t m_state_node;
};

struct plugin_ui_search_graph_edge {
    uint8_t m_is_back;
    plugin_ui_navigation_t m_navigation;
};
    
cpe_graph_t plugin_ui_search_build_graph(plugin_ui_phase_node_t phase, cpe_graph_node_t * cur_state_node);

uint8_t plugin_ui_search_state_name_eq(cpe_graph_node_t node, const void * finish);

#ifdef __cplusplus
}
#endif

#endif

