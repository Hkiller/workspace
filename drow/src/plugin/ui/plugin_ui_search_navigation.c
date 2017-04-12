#include <assert.h>
#include "cpe/utils/graph.h"
#include "cpe/utils/graph_weight_path.h"
#include "plugin_ui_search_i.h"
#include "plugin_ui_env_i.h"
#include "plugin_ui_phase_node_i.h"
#include "plugin_ui_navigation_i.h"
#include "plugin_ui_state_i.h"
#include "plugin_ui_state_node_i.h"

static float plugin_ui_search_next_navigation_weight(cpe_graph_edge_t edge) {
    struct plugin_ui_search_graph_edge * edge_data;
    edge_data = cpe_graph_edge_data(edge);
    return edge_data->m_navigation->m_weight;
}

plugin_ui_navigation_t
plugin_ui_search_next_navigation_to_state(
    plugin_ui_phase_node_t phase, const char * state)
{
    plugin_ui_env_t env = phase->m_env;
    cpe_graph_t graph = NULL;
    cpe_graph_weight_path_t path = NULL;
    cpe_graph_node_t cur_state_node;
    cpe_graph_edge_t use_edge;
    plugin_ui_navigation_t use_navigation;

    graph = plugin_ui_search_build_graph(phase, &cur_state_node);
    if (graph == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_search_next_navigation_to_state: build graph fail!");
        goto SEARCH_ERROR;
    }

    path = cpe_graph_weight_path_create(env->m_module->m_alloc);
    if (path == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_search_next_navigation_to_state: create path fail!");
        goto SEARCH_ERROR;
    }

    if (cpe_graph_shortest_path_dijkstra(
            path,
            graph, cur_state_node, plugin_ui_search_next_navigation_weight,
            state, plugin_ui_search_state_name_eq)
        != 0)
    {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_search_next_navigation_to_state: no path to state %s!",
            state);
        goto SEARCH_ERROR;
    }

    assert(cpe_graph_weight_path_node_begin(path));
    use_edge = cpe_graph_weight_path_node_edge(cpe_graph_weight_path_node_begin(path));
    if (use_edge == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_search_next_navigation_to_state: result path no use edge!");
        goto SEARCH_ERROR;
    }

    use_navigation = ((struct plugin_ui_search_graph_edge *)cpe_graph_edge_data(use_edge))->m_navigation;
    assert(use_navigation);

    if (env->m_debug) {
        CPE_INFO(
            env->m_module->m_em,
            "plugin_ui_search_next_navigation_to_state: from %s to %s: %s",
            plugin_ui_state_node_name(plugin_ui_state_node_current(phase)),
            state,
            cpe_graph_weight_path_dump(gd_app_tmp_buffer(env->m_module->m_app), path));
    }

    cpe_graph_free(graph);
    cpe_graph_weight_path_free(path);

    return use_navigation;

SEARCH_ERROR:
    if (graph) cpe_graph_free(graph);
    if (path) cpe_graph_weight_path_free(path);

    return NULL;
}

