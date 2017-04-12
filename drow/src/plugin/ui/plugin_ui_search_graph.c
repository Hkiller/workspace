#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/graph.h"
#include "plugin_ui_search_graph_i.h"
#include "plugin_ui_navigation_i.h"
#include "plugin_ui_state_node_i.h"
#include "plugin_ui_phase_node_i.h"

static cpe_graph_node_t plugin_ui_graph_add_state_node(plugin_ui_env_t env, cpe_graph_t graph, plugin_ui_state_node_t state_node);
static int plugin_ui_graph_build_back_edges(plugin_ui_env_t env, cpe_graph_t graph, cpe_graph_node_t a_node, cpe_graph_node_t z_node);
static int plugin_ui_graph_build_new_edges(plugin_ui_env_t env, cpe_graph_t graph, cpe_graph_node_t init_node, plugin_ui_state_t skip_state);
static void plugin_ui_graph_node_print(write_stream_t s, cpe_graph_node_t node);
static void plugin_ui_graph_edge_print(write_stream_t s, cpe_graph_edge_t edge);

cpe_graph_t plugin_ui_search_build_graph(plugin_ui_phase_node_t phase, cpe_graph_node_t * cur_state_node) {
    plugin_ui_env_t env = phase->m_env;
    cpe_graph_t graph;
    cpe_graph_node_t cur_node;
    cpe_graph_node_t pre_node;
    plugin_ui_state_node_t state_node;
    plugin_ui_state_node_t state_node_prev;

    graph = cpe_graph_create(
        env->m_module->m_alloc, env->m_module->m_em,
        sizeof(struct plugin_ui_search_graph_node),
        sizeof(struct plugin_ui_search_graph_edge));
    if (graph == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_search_build_graph: alloc graph fail!");
        return NULL;
    }
    cpe_graph_set_print_fun(graph, plugin_ui_graph_node_print, plugin_ui_graph_edge_print);

    state_node = plugin_ui_state_node_current(phase);
    assert(state_node);

    cur_node = plugin_ui_graph_add_state_node(env, graph, state_node);
    if (cur_node == NULL) goto BUILD_GRAPH_ERROR;
    if (plugin_ui_graph_build_new_edges(env, graph, cur_node, NULL) != 0) goto BUILD_GRAPH_ERROR;

    if (cur_state_node) *cur_state_node = cur_node;

    for(state_node_prev = plugin_ui_state_node_prev(state_node);
        state_node_prev;
        cur_node = pre_node,
            state_node = state_node_prev,
            state_node_prev = plugin_ui_state_node_prev(state_node))
    {
        pre_node = plugin_ui_graph_add_state_node(env, graph, state_node_prev);
        if (pre_node == NULL) goto BUILD_GRAPH_ERROR;

        if (plugin_ui_graph_build_back_edges(env, graph, cur_node, pre_node) != 0) goto BUILD_GRAPH_ERROR;
        if (plugin_ui_graph_build_new_edges(
                env, graph, pre_node,
                ((struct plugin_ui_search_graph_node * )cpe_graph_node_data(cur_node))
                ->m_state_node
                ->m_curent.m_process_state)
            != 0) goto BUILD_GRAPH_ERROR;
    }

    return graph;

BUILD_GRAPH_ERROR:
    cpe_graph_free(graph);
    return NULL;
}

static cpe_graph_node_t
plugin_ui_graph_add_state_node(plugin_ui_env_t env, cpe_graph_t graph, plugin_ui_state_node_t state_node) {
    cpe_graph_node_t node;
    struct plugin_ui_search_graph_node * node_data;

    node = cpe_graph_node_create(graph);
    if (node == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_search_build_graph: alloc graph node fail!");
        return NULL;
    }

    node_data = cpe_graph_node_data(node);
    node_data->m_state = plugin_ui_state_node_process_state(state_node);
    node_data->m_state_node = state_node;

    return node;
}

static cpe_graph_node_t
plugin_ui_graph_add_state(plugin_ui_env_t env, cpe_graph_t graph, plugin_ui_state_t state) {
    cpe_graph_node_t node;
    struct plugin_ui_search_graph_node * node_data;

    node = cpe_graph_node_create(graph);
    if (node == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_search_build_graph: alloc graph node fail!");
        return NULL;
    }

    node_data = cpe_graph_node_data(node);
    node_data->m_state = state;
    node_data->m_state_node = NULL;

    return node;
}

static int plugin_ui_graph_add_edge(
    plugin_ui_env_t env, cpe_graph_t graph, cpe_graph_node_t a_node, cpe_graph_node_t z_node, plugin_ui_navigation_t navigation, uint8_t is_back)
{
    cpe_graph_edge_t edge;
    struct plugin_ui_search_graph_edge * edge_data;

    edge = cpe_graph_edge_create(a_node, z_node);
    if (edge == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_search_build_graph: alloc edge node fail!");
        return -1;
    }

    edge_data = cpe_graph_edge_data(edge);
    edge_data->m_navigation = navigation;
    edge_data->m_is_back = is_back;

    /* do { */
    /*     struct plugin_ui_search_graph_node * a_node_data; */
    /*     struct plugin_ui_search_graph_node * z_node_data; */
    /*     a_node_data = cpe_graph_node_data(a_node); */
    /*     z_node_data = cpe_graph_node_data(z_node); */
    /*     CPE_ERROR(env->m_module->m_em, " xxxxx: add edge %s ==> %s", plugin_ui_state_name(a_node_data->m_state), plugin_ui_state_name(z_node_data->m_state)); */
    /* } while(0); */

    return 0;
}

static int plugin_ui_state_cmp(const void * l, const void * r) {
    plugin_ui_state_t l_state = *(plugin_ui_state_t const *)l;
    plugin_ui_state_t r_state = *(plugin_ui_state_t const *)r;

    return l_state == r_state ? 0
        : l_state < r_state ? -1
        : 1;
}

static int plugin_ui_graph_build_new_edges(plugin_ui_env_t env, cpe_graph_t graph, cpe_graph_node_t init_node, plugin_ui_state_t skip_state) {
    struct plugin_ui_navigation_it navigation_it;
    plugin_ui_navigation_t navigation;
    plugin_ui_state_t processed_states[128];
    uint16_t processed_state_count;
    cpe_graph_node_t to_process_nodes[128];
    uint16_t to_process_node_count;

    to_process_node_count = 0;
    to_process_nodes[to_process_node_count++] = init_node;

    processed_state_count = 0;
    if (skip_state) processed_states[processed_state_count++] = skip_state;

    while(to_process_node_count > 0) {
        cpe_graph_node_t a_node = to_process_nodes[--to_process_node_count];
        struct plugin_ui_search_graph_node * a_node_data = cpe_graph_node_data(a_node);

        plugin_ui_state_navigations_to(a_node_data->m_state, &navigation_it);
        while((navigation = plugin_ui_navigation_it_next(&navigation_it))) {
            cpe_graph_node_t z_node;
            plugin_ui_state_t to_state;

            if (navigation->m_state.m_op == plugin_ui_navigation_state_op_back) continue;
            if (navigation->m_state.m_op == plugin_ui_navigation_state_op_template) continue;

            to_state = navigation->m_to_state;
            assert(to_state);
            
            if (bsearch(&to_state, processed_states, processed_state_count, sizeof(processed_states[0]), plugin_ui_state_cmp) != NULL) {
                continue;
            }

            z_node = plugin_ui_graph_add_state(env, graph, to_state);
            if (z_node == NULL) return -1;

            if (plugin_ui_graph_add_edge(env, graph, a_node, z_node, navigation, 0) != 0) return -1;

            if (to_process_node_count >= CPE_ARRAY_SIZE(to_process_nodes)) {
                CPE_ERROR(env->m_module->m_em, "plugin_ui_search_build_graph: to_process_nodes overflow!");
                return -1;
            }
            to_process_nodes[to_process_node_count++] = z_node;
            
            if (processed_state_count >= CPE_ARRAY_SIZE(processed_states)) {
                CPE_ERROR(env->m_module->m_em, "plugin_ui_search_build_graph: process_states overflow!");
                return -1;
            }
            processed_states[processed_state_count++] = to_state;
            qsort(processed_states, processed_state_count, sizeof(processed_states[0]), plugin_ui_state_cmp);
        }
    }

    return 0;
}

static int plugin_ui_graph_build_back_edges(plugin_ui_env_t env, cpe_graph_t graph, cpe_graph_node_t a_node, cpe_graph_node_t z_node) {
    struct plugin_ui_navigation_it navigation_it;
    plugin_ui_navigation_t navigation;
    struct plugin_ui_search_graph_node * a_node_data;

    a_node_data = cpe_graph_node_data(a_node);

    plugin_ui_state_navigations_to(a_node_data->m_state, &navigation_it);

    while((navigation = plugin_ui_navigation_it_next(&navigation_it))) {
        //TODO:
        if (navigation->m_category != plugin_ui_navigation_category_state) continue;
        
        if (navigation->m_state.m_op == plugin_ui_navigation_state_op_back) {
            if (plugin_ui_graph_add_edge(env, graph, a_node, z_node, navigation, 1) != 0) return -1;
        }
    }

    return 0;
}

static void plugin_ui_graph_node_print(write_stream_t s, cpe_graph_node_t node) {
    struct plugin_ui_search_graph_node * node_data = cpe_graph_node_data(node);
    stream_printf(s, "%s", plugin_ui_state_name(node_data->m_state));
}

static void plugin_ui_graph_edge_print(write_stream_t s, cpe_graph_edge_t edge) {
    struct plugin_ui_search_graph_edge * edge_data = cpe_graph_edge_data(edge);
    stream_printf(s, "%s", edge_data->m_navigation->m_trigger_control);
}

uint8_t plugin_ui_search_state_name_eq(cpe_graph_node_t node, const void * finish) {
    struct plugin_ui_search_graph_node * graph_node = cpe_graph_node_data(node);
    return strcmp(plugin_ui_state_name(graph_node->m_state), finish) == 0 ? 1 : 0;
}

