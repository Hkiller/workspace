#ifndef CPE_UTILS_GRAPH_I_H
#define CPE_UTILS_GRAPH_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/graph.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(cpe_graph_node_list, cpe_graph_node) cpe_graph_node_list_t;
typedef TAILQ_HEAD(cpe_graph_edge_list, cpe_graph_edge) cpe_graph_edge_list_t;

struct cpe_graph {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    cpe_graph_node_print_fun_t m_node_print;
    cpe_graph_edge_print_fun_t m_edge_print;
    uint16_t m_node_capacity;
    uint16_t m_edge_capacity;
    uint32_t m_node_count;
    cpe_graph_node_list_t m_nodes;
    uint32_t m_edge_count;
    cpe_graph_edge_list_t m_edges;
};

#ifdef __cplusplus
}
#endif

#endif

