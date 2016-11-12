#ifndef CPE_UTILS_GRAPH_H
#define CPE_UTILS_GRAPH_H
#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*cpe_graph_node_print_fun_t)(write_stream_t s, cpe_graph_node_t node);
typedef void (*cpe_graph_edge_print_fun_t)(write_stream_t s, cpe_graph_edge_t edge);
    
typedef float (*cpe_graph_edge_weight_fun_t)(cpe_graph_edge_t edge);
typedef uint8_t (*cpe_graph_node_check_fun_t)(cpe_graph_node_t node, const void * finish);

cpe_graph_t cpe_graph_create(mem_allocrator_t alloc, error_monitor_t em, uint16_t node_capacity, uint16_t edge_capacity);
void cpe_graph_free(cpe_graph_t graph);
uint16_t cpe_graph_node_capacity(cpe_graph_t graph);
uint16_t cpe_graph_edge_capacity(cpe_graph_t graph);    
uint32_t cpe_graph_node_count(cpe_graph_t graph);
uint32_t cpe_graph_edge_count(cpe_graph_t graph);    
void cpe_graph_set_print_fun(cpe_graph_t graph, cpe_graph_node_print_fun_t node_print, cpe_graph_edge_print_fun_t edge_print);

/*node*/
cpe_graph_node_t cpe_graph_node_create(cpe_graph_t graph);
void cpe_graph_node_free(cpe_graph_node_t node);
void cpe_graph_node_print(write_stream_t s, cpe_graph_node_t node);

uint32_t cpe_graph_node_edge_count(cpe_graph_node_t node);
uint32_t cpe_graph_node_edge_count_as_a(cpe_graph_node_t node);
uint32_t cpe_graph_node_edge_count_as_z(cpe_graph_node_t node);
void * cpe_graph_node_data(cpe_graph_node_t node);
cpe_graph_node_t cpe_graph_node_from_data(void * node_data);

/*node_it*/    
struct cpe_graph_node_it {
    cpe_graph_node_t (*next)(struct cpe_graph_node_it * it);
    void * m_data;
};
void cpe_graph_nodes(cpe_graph_node_it_t it, cpe_graph_t graph);
void cpe_graph_node_edges_as_a(cpe_graph_edge_it_t it, cpe_graph_node_t node);
void cpe_graph_node_edges_as_z(cpe_graph_edge_it_t it, cpe_graph_node_t node);

#define cpe_graph_node_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
/*edge*/
cpe_graph_edge_t cpe_graph_edge_create(cpe_graph_node_t a, cpe_graph_node_t z);
void cpe_graph_edge_free(cpe_graph_edge_t edge);
void cpe_graph_edge_print(write_stream_t s, cpe_graph_edge_t dege);
void * cpe_graph_edge_data(cpe_graph_edge_t edge);
cpe_graph_edge_t cpe_graph_edge_from_data(void * edge_data);

cpe_graph_node_t cpe_graph_edge_a(cpe_graph_edge_t edge);
cpe_graph_node_t cpe_graph_edge_z(cpe_graph_edge_t edge);
    
/*edge_it*/    
struct cpe_graph_edge_it {
    cpe_graph_edge_t (*next)(struct cpe_graph_edge_it * it);
    void * m_data;
};
void cpe_graph_edges(cpe_graph_edge_it_t it, cpe_graph_t graph);
#define cpe_graph_edge_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*algorithm */
int cpe_graph_shortest_path_dijkstra(
    cpe_graph_weight_path_t o_path,
    cpe_graph_t graph, cpe_graph_node_t start, cpe_graph_edge_weight_fun_t weight_fun,
    const void * finish, cpe_graph_node_check_fun_t finish_check);
    
#ifdef __cplusplus
}
#endif

#endif
