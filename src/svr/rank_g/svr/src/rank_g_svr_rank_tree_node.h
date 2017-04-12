#ifndef SVR_RANK_G_SVR_RANKTREE_NODE_H
#define SVR_RANK_G_SVR_RANKTREE_NODE_H
#include "rank_g_svr.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    rt_color_red = 0
    , rt_color_black = 1
};

struct rt_node {
    uint32_t m_parent; /*m_next when in free list*/
    uint32_t m_child[2];
    uint32_t m_order;
    uint32_t m_color;
    uint32_t m_value;
    uint32_t m_record_id;
};

#define rt_node_value(_node) ((_node)->m_value)
#define rt_node_record_id(_node) ((_node)->m_record_id)

#ifdef __cplusplus
}
#endif

#endif
