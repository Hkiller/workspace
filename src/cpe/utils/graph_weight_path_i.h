#ifndef CPE_UTILS_GRAPH_PATH_NODE_I_H
#define CPE_UTILS_GRAPH_PATH_NODE_I_H
#include "cpe/utils/graph_weight_path.h"
#include "graph_node_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(cpe_graph_weight_path_node_list, cpe_graph_weight_path_node) cpe_graph_weight_path_node_list_t;

struct cpe_graph_weight_path_node {
    TAILQ_ENTRY(cpe_graph_weight_path_node) m_next;
    cpe_graph_node_t m_node;
    cpe_graph_edge_t m_edge;
    float m_weight;
};

struct cpe_graph_weight_path {
    mem_allocrator_t m_alloc;
    cpe_graph_weight_path_node_list_t m_nodes;
    cpe_graph_weight_path_node_list_t m_free_nodes;
};

cpe_graph_weight_path_node_t cpe_graph_weight_path_node_alloc(cpe_graph_weight_path_t weight_path);

#ifdef __cplusplus
}
#endif

#endif

