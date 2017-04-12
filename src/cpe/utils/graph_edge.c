#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream.h"
#include "cpe/utils/string_utils.h"
#include "graph_edge_i.h"
#include "graph_node_i.h"

cpe_graph_edge_t cpe_graph_edge_create(cpe_graph_node_t a, cpe_graph_node_t z) {
    cpe_graph_t graph = a->m_graph;
    cpe_graph_edge_t edge;

    assert(a);
    assert(z);

    edge = mem_alloc(graph->m_alloc, sizeof(struct cpe_graph_edge) + graph->m_edge_capacity);
    if (edge == NULL) {
        CPE_ERROR(graph->m_em, "cpe_graph_edge_create: alloc fail!");
        return NULL;
    }

    edge->m_a = a;
    a->m_edge_count_as_a++;
    TAILQ_INSERT_TAIL(&a->m_edges_as_a, edge, m_next_for_node_a);

    edge->m_z = z;
    z->m_edge_count_as_z++;
    TAILQ_INSERT_TAIL(&z->m_edges_as_z, edge, m_next_for_node_z);

    graph->m_edge_count++;    
    TAILQ_INSERT_TAIL(&graph->m_edges, edge, m_next_for_graph);
    
    return edge;
}

void cpe_graph_edge_print(write_stream_t s, cpe_graph_edge_t edge) {
    if (edge->m_a->m_graph->m_edge_print) {
        edge->m_a->m_graph->m_edge_print(s, edge);
    }
    else {
        stream_printf(s, "%p", edge);
    }
}

void * cpe_graph_edge_data(cpe_graph_edge_t edge) {
    return edge + 1;
}

cpe_graph_edge_t cpe_graph_edge_from_data(void * edge_data) {
    return ((cpe_graph_edge_t)edge_data) - 1;
}

cpe_graph_node_t cpe_graph_edge_a(cpe_graph_edge_t edge) {
    return edge->m_a;
}

cpe_graph_node_t cpe_graph_edge_z(cpe_graph_edge_t edge) {
    return edge->m_z;
}

void cpe_graph_edge_free(cpe_graph_edge_t edge) {
    cpe_graph_t graph = edge->m_a->m_graph;

    edge->m_a->m_edge_count_as_a--;
    TAILQ_REMOVE(&edge->m_a->m_edges_as_a, edge, m_next_for_node_a);

    edge->m_z->m_edge_count_as_z--;
    TAILQ_REMOVE(&edge->m_z->m_edges_as_z, edge, m_next_for_node_z);

    graph->m_edge_count--;
    TAILQ_REMOVE(&graph->m_edges, edge, m_next_for_graph);

    mem_free(graph->m_alloc, edge);
}

static cpe_graph_edge_t cpe_graph_edge_next(cpe_graph_edge_it_t it) {
    cpe_graph_edge_t r;
    if (it->m_data == NULL) return NULL;
    r = it->m_data;
    it->m_data = TAILQ_NEXT(r, m_next_for_graph);
    return r;
}

void cpe_graph_edges(cpe_graph_edge_it_t it, cpe_graph_t graph) {
    it->m_data = TAILQ_FIRST(&graph->m_edges);
    it->next = cpe_graph_edge_next;
}

