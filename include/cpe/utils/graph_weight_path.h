#ifndef CPE_UTILS_GRAPH_WEIGHT_PATH_H
#define CPE_UTILS_GRAPH_WEIGHT_PATH_H
#include "graph.h"

#ifdef __cplusplus
extern "C" {
#endif

cpe_graph_weight_path_t cpe_graph_weight_path_create(mem_allocrator_t alloc);
void cpe_graph_weight_path_free(cpe_graph_weight_path_t weight_path);

void cpe_graph_weight_path_clear(cpe_graph_weight_path_t weight_path);

cpe_graph_weight_path_node_t cpe_graph_weight_path_node_set_start(cpe_graph_weight_path_t weight_path);
cpe_graph_weight_path_node_t
cpe_graph_weight_path_node_append(
    cpe_graph_weight_path_t weight_path, cpe_graph_node_t node, cpe_graph_edge_t edge, float weight);
    
cpe_graph_weight_path_node_t cpe_graph_weight_path_node_begin(cpe_graph_weight_path_t weight_path);
cpe_graph_weight_path_node_t cpe_graph_weight_path_node_finish(cpe_graph_weight_path_t weight_path);
cpe_graph_weight_path_node_t cpe_graph_weight_path_node_prev(cpe_graph_weight_path_node_t path_node);
cpe_graph_weight_path_node_t cpe_graph_weight_path_node_next(cpe_graph_weight_path_node_t path_node);

cpe_graph_node_t cpe_graph_weight_path_node_node(cpe_graph_weight_path_node_t path_node);
cpe_graph_edge_t cpe_graph_weight_path_node_edge(cpe_graph_weight_path_node_t path_node);
float cpe_graph_weight_path_node_weight(cpe_graph_weight_path_node_t path_node);

void cpe_graph_weight_path_print(write_stream_t s, cpe_graph_weight_path_t path);
char * cpe_graph_weight_path_dump(mem_buffer_t buffer, cpe_graph_weight_path_t path);

#ifdef __cplusplus
}
#endif

#endif
