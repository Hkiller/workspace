#include <assert.h>
#include "cpe/utils/stream_buffer.h"
#include "graph_weight_path_i.h"
#include "graph_node_i.h"
#include "graph_edge_i.h"
#include "graph_i.h"

cpe_graph_weight_path_t cpe_graph_weight_path_create(mem_allocrator_t alloc) {
    cpe_graph_weight_path_t path;

    path = mem_alloc(alloc, sizeof(struct cpe_graph_weight_path));
    if (path == NULL) return NULL;

    path->m_alloc = alloc;
    TAILQ_INIT(&path->m_nodes);
    TAILQ_INIT(&path->m_free_nodes);

    return path;
}

void cpe_graph_weight_path_free(cpe_graph_weight_path_t weight_path) {
    cpe_graph_weight_path_clear(weight_path);
    assert(TAILQ_EMPTY(&weight_path->m_nodes));

    while(!TAILQ_EMPTY(&weight_path->m_free_nodes)) {
        cpe_graph_weight_path_node_t n = TAILQ_FIRST(&weight_path->m_free_nodes);
        TAILQ_REMOVE(&weight_path->m_free_nodes, n, m_next);
        mem_free(weight_path->m_alloc, n);
    }

    mem_free(weight_path->m_alloc, weight_path);
}

void cpe_graph_weight_path_clear(cpe_graph_weight_path_t weight_path) {
    while(!TAILQ_EMPTY(&weight_path->m_nodes)) {
        cpe_graph_weight_path_node_t n = TAILQ_FIRST(&weight_path->m_nodes);
        TAILQ_REMOVE(&weight_path->m_nodes, n, m_next);
        TAILQ_INSERT_TAIL(&weight_path->m_free_nodes, n, m_next);
    }
}

cpe_graph_weight_path_node_t cpe_graph_weight_path_node_alloc(cpe_graph_weight_path_t weight_path) {
    cpe_graph_weight_path_node_t n;

    n = TAILQ_FIRST(&weight_path->m_free_nodes);
    if (n) {
        TAILQ_REMOVE(&weight_path->m_free_nodes, n, m_next);
    }
    else {
        n = mem_alloc(weight_path->m_alloc, sizeof(struct cpe_graph_weight_path_node));
        if (n == NULL) return NULL; 
    }

    return n;
}

cpe_graph_weight_path_node_t cpe_graph_weight_path_set_start(cpe_graph_weight_path_t weight_path, cpe_graph_node_t node) {
    cpe_graph_weight_path_node_t n;

    assert(TAILQ_EMPTY(&weight_path->m_nodes));

    n = cpe_graph_weight_path_node_alloc(weight_path);
    if (n == NULL) return NULL; 

    n->m_node = node;
    n->m_edge = NULL;
    n->m_weight = 0.0f;

    TAILQ_INSERT_TAIL(&weight_path->m_nodes, n, m_next);

    return n;
}

cpe_graph_weight_path_node_t
cpe_graph_weight_path_node_append(
    cpe_graph_weight_path_t weight_path, cpe_graph_node_t node, cpe_graph_edge_t edge, float weight)
{
    cpe_graph_weight_path_node_t n;
    cpe_graph_weight_path_node_t from;    

    from = TAILQ_LAST(&weight_path->m_nodes, cpe_graph_weight_path_node_list);
    if (from == NULL) return NULL;
    
    n = cpe_graph_weight_path_node_alloc(weight_path);
    if (n == NULL) return NULL; 

    from->m_edge = edge;
    
    n->m_node = node;
    n->m_edge = NULL;
    n->m_weight = from->m_weight + weight;

    TAILQ_INSERT_TAIL(&weight_path->m_nodes, n, m_next);

    return n;
}
    
cpe_graph_weight_path_node_t cpe_graph_weight_path_node_begin(cpe_graph_weight_path_t weight_path) {
    return TAILQ_FIRST(&weight_path->m_nodes);
}

cpe_graph_weight_path_node_t cpe_graph_weight_path_node_finish(cpe_graph_weight_path_t weight_path) {
    return TAILQ_LAST(&weight_path->m_nodes, cpe_graph_weight_path_node_list);
}

cpe_graph_weight_path_node_t cpe_graph_weight_path_node_prev(cpe_graph_weight_path_node_t path_node) {
    return TAILQ_PREV(path_node, cpe_graph_weight_path_node_list, m_next);
}

cpe_graph_weight_path_node_t cpe_graph_weight_path_node_next(cpe_graph_weight_path_node_t path_node) {
    return TAILQ_NEXT(path_node, m_next);
}

cpe_graph_node_t cpe_graph_weight_path_node_node(cpe_graph_weight_path_node_t path_node) {
    return path_node->m_node;
}

cpe_graph_edge_t cpe_graph_weight_path_node_edge(cpe_graph_weight_path_node_t path_node) {
    return path_node->m_edge;
}

float cpe_graph_weight_path_node_weight(cpe_graph_weight_path_node_t path_node) {
    return path_node->m_weight;
}

void cpe_graph_weight_path_print(
    write_stream_t s, cpe_graph_weight_path_t path)
{
    cpe_graph_weight_path_node_t n;

    TAILQ_FOREACH(n, &path->m_nodes, m_next) {
        cpe_graph_node_print(s, n->m_node);

        if ((float)((int)n->m_weight) == n->m_weight) {
            stream_printf(s, "[%d]", (int)n->m_weight);
        }
        else {
            stream_printf(s, "[%f]", n->m_weight);
        }
        
        if (n->m_edge) {
            stream_printf(s, "-(");
            cpe_graph_edge_print(s, n->m_edge);
            stream_printf(s, ")->");
        }
    }
}

char * cpe_graph_weight_path_dump(mem_buffer_t buffer, cpe_graph_weight_path_t path) {
    struct write_stream_buffer ws = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);
    cpe_graph_weight_path_print((write_stream_t)&ws, path);
    mem_buffer_append_char(buffer, 0);

    return (char *)mem_buffer_make_continuous(buffer, 0);
}
