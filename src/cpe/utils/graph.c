#include <assert.h>
#include "graph_i.h"
#include "graph_node_i.h"
#include "graph_edge_i.h"

cpe_graph_t cpe_graph_create(mem_allocrator_t alloc, error_monitor_t em, uint16_t node_capacity, uint16_t edge_capacity) {
    cpe_graph_t graph;

    graph = mem_alloc(alloc, sizeof(struct cpe_graph));
    if (graph == NULL) {
        CPE_ERROR(em, "cpe_graph_create: alloc fail!");
        return NULL;
    }

    graph->m_alloc = alloc;
    graph->m_em = em;
    graph->m_node_print = NULL;
    graph->m_edge_print = NULL;
    graph->m_node_capacity = node_capacity;
    graph->m_edge_capacity = edge_capacity;
    graph->m_node_count = 0;
    graph->m_edge_count = 0;
    TAILQ_INIT(&graph->m_nodes);
    TAILQ_INIT(&graph->m_edges);

    return graph;
}

void cpe_graph_free(cpe_graph_t graph) {
    while(!TAILQ_EMPTY(&graph->m_nodes)) {
        cpe_graph_node_free(TAILQ_FIRST(&graph->m_nodes));
    }

    assert(TAILQ_EMPTY(&graph->m_edges));
    assert(graph->m_node_count == 0);
    assert(graph->m_edge_count == 0);

    mem_free(graph->m_alloc, graph);
}

uint16_t cpe_graph_node_capacity(cpe_graph_t graph) {
    return graph->m_node_capacity;
}

uint16_t cpe_graph_edge_capacity(cpe_graph_t graph) {
    return graph->m_edge_capacity;
}

uint32_t cpe_graph_node_count(cpe_graph_t graph) {
    return graph->m_node_count;
}

uint32_t cpe_graph_edge_count(cpe_graph_t graph) {
    return graph->m_edge_count;
}

void cpe_graph_set_print_fun(cpe_graph_t graph, cpe_graph_node_print_fun_t node_print, cpe_graph_edge_print_fun_t edge_print) {
    graph->m_node_print = node_print;
    graph->m_edge_print = edge_print;
}
