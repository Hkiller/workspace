#include "graph_node_i.h"
#include "graph_edge_i.h"
#include "graph_weight_path_i.h"

typedef struct cpe_graph_dijkstra_path_node * cpe_graph_dijkstra_path_node_t;
typedef struct cpe_graph_dijkstra_path_node_heap * cpe_graph_dijkstra_path_node_heap_t;

struct cpe_graph_dijkstra_path_node {
    cpe_graph_node_t m_node;
    cpe_graph_edge_t m_edge;
    cpe_graph_dijkstra_path_node_t m_via;	/* where previous node is in shortest path */
	float m_dist;	/* distance from origining node */
	int m_heap_idx;	/* link to heap position for updating distance */
};

struct cpe_graph_dijkstra_path_node_heap {
    uint16_t m_capacity;
    uint16_t m_count;
    cpe_graph_dijkstra_path_node_t * m_nodes;
};

static cpe_graph_dijkstra_path_node_heap_t
cpe_graph_dijkstra_path_node_heap_create(cpe_graph_t graph, uint16_t capacity) {
    cpe_graph_dijkstra_path_node_heap_t heap;
    struct cpe_graph_dijkstra_path_node * dijkstra_path_nodes;
    cpe_graph_node_t node;

    heap = mem_alloc(
        graph->m_alloc,
        sizeof(struct cpe_graph_dijkstra_path_node_heap)
        + sizeof(struct cpe_graph_dijkstra_path_node) * graph->m_node_count
        + sizeof(cpe_graph_dijkstra_path_node_t) * capacity);
    if (heap == NULL) return NULL;

    dijkstra_path_nodes = (void*)(heap + 1);

    TAILQ_FOREACH(node, &graph->m_nodes, m_next) {
        node->m_extern = dijkstra_path_nodes;
        dijkstra_path_nodes->m_node = node;
        dijkstra_path_nodes->m_edge = NULL;
        dijkstra_path_nodes->m_via = NULL;
        dijkstra_path_nodes->m_dist = 0.0f;
        dijkstra_path_nodes->m_heap_idx = 0;
        dijkstra_path_nodes++;
    }

    heap->m_capacity = capacity;
    heap->m_count = 0;
    heap->m_nodes = (void*)(dijkstra_path_nodes);

    return  heap;
}

static void cpe_graph_dijkstra_path_node_heap_free(cpe_graph_t graph, cpe_graph_dijkstra_path_node_heap_t heap) {
    cpe_graph_node_t node;

    TAILQ_FOREACH(node, &graph->m_nodes, m_next) {
        node->m_extern = NULL;
    }
    
    mem_free(graph->m_alloc, heap);
}

static void cpe_graph_dijkstra_path_node_heap_set_dist(
    cpe_graph_dijkstra_path_node_heap_t heap,
    cpe_graph_dijkstra_path_node_t node, cpe_graph_dijkstra_path_node_t via, cpe_graph_edge_t edge, float d)
{
	int i, j;
 
	if (node->m_via && d >= node->m_dist) return;
 
	node->m_dist = d;
    node->m_edge = edge;
	node->m_via = via;
 
	i = node->m_heap_idx;
	if (!i) i = ++heap->m_count;
 
	/* upheap */
	for (; i > 1 && node->m_dist < heap->m_nodes[j = i/2]->m_dist; i = j) {
		(heap->m_nodes[i] = heap->m_nodes[j])->m_heap_idx = i;
    }

	heap->m_nodes[i] = node;
	node->m_heap_idx = i;
}

static cpe_graph_dijkstra_path_node_t
cpe_graph_dijkstra_path_node_heap_pop(cpe_graph_dijkstra_path_node_heap_t heap) {
	cpe_graph_dijkstra_path_node_t node, tmp;
	int i, j;
 
	if (!heap->m_count) return NULL;
 
	/* remove leading element, pull tail element there and downheap */
	node = heap->m_nodes[1];
	tmp = heap->m_nodes[heap->m_count--];
 
	for (i = 1; i < heap->m_count && (j = i * 2) <= heap->m_count; i = j) {
		if (j < heap->m_count && heap->m_nodes[j]->m_dist > heap->m_nodes[j+1]->m_dist) j++;
 
		if (heap->m_nodes[j]->m_dist >= tmp->m_dist) break;
		(heap->m_nodes[i] = heap->m_nodes[j])->m_heap_idx = i;
	}
 
	heap->m_nodes[i] = tmp;
	tmp->m_heap_idx = i;

	return node;
}

static int cpe_graph_dijkstra_node_build_path(cpe_graph_weight_path_t o_path, cpe_graph_dijkstra_path_node_t node) {
    cpe_graph_weight_path_node_t pn;

    cpe_graph_weight_path_clear(o_path);

    pn = cpe_graph_weight_path_node_alloc(o_path);
    if (pn == NULL) return -1; 

    pn->m_node = node->m_node;
    pn->m_edge = NULL;
    pn->m_weight = node->m_dist;
    TAILQ_INSERT_TAIL(&o_path->m_nodes, pn, m_next);

    while(node->m_via) {
        cpe_graph_weight_path_node_t from_pn = cpe_graph_weight_path_node_alloc(o_path);
        if (from_pn == NULL) return -1;

        from_pn->m_node = node->m_via->m_node;
        from_pn->m_edge = node->m_edge;
        from_pn->m_weight = node->m_via->m_dist;

        TAILQ_INSERT_HEAD(&o_path->m_nodes, from_pn, m_next);

        node = node->m_via;
    }

    return 0;
}
    
int cpe_graph_shortest_path_dijkstra(
    cpe_graph_weight_path_t o_path,
    cpe_graph_t graph, cpe_graph_node_t start, cpe_graph_edge_weight_fun_t weight_fun,
    const void * finish, cpe_graph_node_check_fun_t finish_check)
{
    cpe_graph_dijkstra_path_node_heap_t heap = NULL;
	cpe_graph_dijkstra_path_node_t lead;
    cpe_graph_dijkstra_path_node_t target = NULL;
    int rv = -1;
    
    heap = cpe_graph_dijkstra_path_node_heap_create(graph, graph->m_node_count + 1);
    if (heap == NULL) {
        CPE_ERROR(graph->m_em, "cpe_graph_shortest_path_dijkstra: create heap fail!");
        goto OP_ERROR;
    }

    cpe_graph_dijkstra_path_node_heap_set_dist(heap, start->m_extern, NULL, NULL, 0);
    
	while((lead = cpe_graph_dijkstra_path_node_heap_pop(heap))) {
        cpe_graph_edge_t edge;

        TAILQ_FOREACH(edge, &lead->m_node->m_edges_as_a, m_next_for_node_a) {
            float dist = weight_fun(edge);
            cpe_graph_dijkstra_path_node_t to = (cpe_graph_dijkstra_path_node_t)edge->m_z->m_extern;
            
			cpe_graph_dijkstra_path_node_heap_set_dist(heap, to, lead, edge, lead->m_dist + dist);

            if ((finish_check ? finish_check(to->m_node, finish) : (to->m_node == finish ? 1 : 0))) {
                if (target == NULL || to->m_dist < target->m_dist) target = to;
            }
        }
    }

    if (target) {
        if (o_path) {
            rv = cpe_graph_dijkstra_node_build_path(o_path, target);
        }
        else {
            rv = 0;
        }
    }

    cpe_graph_dijkstra_path_node_heap_free(graph, heap);
    
    return rv;

OP_ERROR:
    if (heap) cpe_graph_dijkstra_path_node_heap_free(graph, heap);
    return rv;
}
