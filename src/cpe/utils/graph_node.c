#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream.h"
#include "cpe/utils/string_utils.h"
#include "graph_node_i.h"
#include "graph_edge_i.h"

cpe_graph_node_t cpe_graph_node_create(cpe_graph_t graph) {
    cpe_graph_node_t node;

    node = mem_alloc(graph->m_alloc, sizeof(struct cpe_graph_node) + graph->m_node_capacity);
    if (node == NULL) {
        CPE_ERROR(graph->m_em, "cpe_graph_node_create: alloc fail!");
        return NULL;
    }

    node->m_graph = graph;
    node->m_edge_count_as_a = 0;
    node->m_edge_count_as_z = 0;
    TAILQ_INIT(&node->m_edges_as_a);
    TAILQ_INIT(&node->m_edges_as_z);
    node->m_extern = NULL;

    graph->m_node_count++;
    TAILQ_INSERT_TAIL(&graph->m_nodes, node, m_next);
    
    return node;
}

void cpe_graph_node_free(cpe_graph_node_t node) {
    while(!TAILQ_EMPTY(&node->m_edges_as_a)) {
        cpe_graph_edge_free(TAILQ_FIRST(&node->m_edges_as_a));
    }
    assert(node->m_edge_count_as_a == 0);

    while(!TAILQ_EMPTY(&node->m_edges_as_z)) {
        cpe_graph_edge_free(TAILQ_FIRST(&node->m_edges_as_z));
    }
    assert(node->m_edge_count_as_z == 0);
    
    node->m_graph->m_node_count--;
    TAILQ_REMOVE(&node->m_graph->m_nodes, node, m_next);
    
    mem_free(node->m_graph->m_alloc, node);
}

void cpe_graph_node_print(write_stream_t s, cpe_graph_node_t node) {
    if (node->m_graph->m_node_print) {
        node->m_graph->m_node_print(s, node);
    }
    else {
        stream_printf(s, "%p", node);
    }
}

uint32_t cpe_graph_node_edge_count(cpe_graph_node_t node) {
    return node->m_edge_count_as_a + node->m_edge_count_as_z;
}

uint32_t cpe_graph_node_edge_count_as_a(cpe_graph_node_t node) {
    return node->m_edge_count_as_a;
}

uint32_t cpe_graph_node_edge_count_as_z(cpe_graph_node_t node) {
    return node->m_edge_count_as_z;
}

void * cpe_graph_node_data(cpe_graph_node_t node) {
    return node + 1;
}

cpe_graph_node_t cpe_graph_node_from_data(void * node_data) {
    return ((cpe_graph_node_t)node_data) - 1;
}

static cpe_graph_node_t cpe_graph_node_next(cpe_graph_node_it_t it) {
    cpe_graph_node_t r;
    if (it->m_data == NULL) return NULL;
    r = it->m_data;
    it->m_data = TAILQ_NEXT(r, m_next);
    return r;
}

void cpe_graph_nodes(cpe_graph_node_it_t it, cpe_graph_t graph) {
    it->m_data = TAILQ_FIRST(&graph->m_nodes);
    it->next = cpe_graph_node_next;
}

static cpe_graph_edge_t cpe_graph_node_edges_as_a_next(cpe_graph_edge_it_t it) {
    cpe_graph_edge_t r;
    if (it->m_data == NULL) return NULL;
    r = it->m_data;
    it->m_data = TAILQ_NEXT(r, m_next_for_node_a);
    return r;
}

void cpe_graph_node_edges_as_a(cpe_graph_edge_it_t it, cpe_graph_node_t node) {
    it->m_data = TAILQ_FIRST(&node->m_edges_as_a);
    it->next = cpe_graph_node_edges_as_a_next;
}

static cpe_graph_edge_t cpe_graph_node_edges_as_z_next(cpe_graph_edge_it_t it) {
    cpe_graph_edge_t r;
    if (it->m_data == NULL) return NULL;
    r = it->m_data;
    it->m_data = TAILQ_NEXT(r, m_next_for_node_z);
    return r;
}

void cpe_graph_node_edges_as_z(cpe_graph_edge_it_t it, cpe_graph_node_t node) {
    it->m_data = TAILQ_FIRST(&node->m_edges_as_z);
    it->next = cpe_graph_node_edges_as_z_next;
}

