#ifndef CPE_UTILS_GRAPH_NODE_I_H
#define CPE_UTILS_GRAPH_NODE_I_H
#include "graph_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cpe_graph_node {
    cpe_graph_t m_graph;
    TAILQ_ENTRY(cpe_graph_node) m_next;
    uint32_t m_edge_count_as_a;
    cpe_graph_edge_list_t m_edges_as_a;
    uint32_t m_edge_count_as_z;
    cpe_graph_edge_list_t m_edges_as_z;
    void * m_extern;
};
    
#ifdef __cplusplus
}
#endif

#endif

