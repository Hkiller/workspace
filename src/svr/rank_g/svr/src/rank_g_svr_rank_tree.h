#ifndef SVR_RANK_G_SVR_RANKTREE_H
#define SVR_RANK_G_SVR_RANKTREE_H
#include "rank_g_svr_rank_tree_node.h"

#ifdef __cplusplus
extern "C" {
#endif

struct rt_head {
    uint32_t m_node_capacity;
    uint32_t m_root;
    uint32_t m_free_node;
};

struct rt {
    rank_g_svr_t m_svr;
    struct rt_head * m_head;
    rt_node_t m_nodes;
};


/*rank tree*/
#define INVALID_RANK_TREE_NODE_POS ((uint32_t)-1)

size_t rt_buff_calc_capacity(uint32_t node_count);
int rt_buff_init(error_monitor_t em, uint32_t node_count, void * buff, size_t buff_capacity);
rt_t rt_create(rank_g_svr_t svr, void * buff, size_t buff_capacity);
void rt_free(rt_t rank_tree);
void rt_tree_clear(rt_t rank_tree);
    
#define rt_node_get(__tree, __idx) ((__idx) == INVALID_RANK_TREE_NODE_POS ? NULL : __tree->m_nodes + (__idx))
#define rt_node_idx(__tree, __node) (__node ? (__node - __tree->m_nodes) : INVALID_RANK_TREE_NODE_POS)
    
rt_node_t rt_find_by_rank(rt_t rank_tree, uint32_t rank_idx);
rt_node_t rt_pre(rt_t rank_tree, rt_node_t node);
rt_node_t rt_next(rt_t rank_tree, rt_node_t node);
rt_node_t rt_insert(rt_t rank_tree, uint32_t value, uint32_t record_id);
void rt_erase(rt_t rank_tree, rt_node_t node);
uint32_t rt_size(rt_t rank_tree);
uint32_t rt_capacity(rt_t rank_tree);
rt_node_t rt_last(rt_t rank_tree);
rt_node_t rt_first(rt_t rank_tree);

rt_node_t rt_find_by_value_min(rt_t rank_tree, uint32_t value);
rt_node_t rt_find_by_value_max(rt_t rank_tree, uint32_t value);
    
uint32_t rt_node_rank(rt_t rank_tree, rt_node_t node);

const char * rt_dump(rt_t rank_tree, mem_buffer_t buffer);

#ifdef __cplusplus
}
#endif

#endif
