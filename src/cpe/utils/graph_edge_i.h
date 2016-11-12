#ifndef CPE_UTILS_GRAPH_EDGE_I_H
#define CPE_UTILS_GRAPH_EDGE_I_H
#include "graph_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cpe_graph_edge {
    TAILQ_ENTRY(cpe_graph_edge) m_next_for_graph;
    cpe_graph_node_t m_a;
    TAILQ_ENTRY(cpe_graph_edge) m_next_for_node_a;
    cpe_graph_node_t m_z;
    TAILQ_ENTRY(cpe_graph_edge) m_next_for_node_z;
};

#ifdef __cplusplus
}
#endif

#endif

