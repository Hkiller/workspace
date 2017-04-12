#ifndef SVR_RANK_G_SVR_TEST_RANKTREE_H
#define SVR_RANK_G_SVR_TEST_RANKTREE_H
#include "RankGSvrTest.hpp"
#include "../rank_g_svr_rank_tree.h"


typedef ::Loki::NullType RankTreeTestBase;

class RankTreeTest : public testenv::fixture<RankTreeTestBase, RankGSvrTest> {
public:
    void SetUp();
    void TearDown();

    void t_rank_tree_create(uint32_t node_count);
    void t_rank_tree_install(uint32_t const * values, uint32_t value_count);
    void t_rank_tree_erase(rt_node_t node);
    
    const char * range(uint32_t value);
    const char * range();
    const char * values();    
    rt_node_t find_min(uint32_t value);
    rt_node_t find_max(uint32_t value);
    rt_node_t get_at(uint32_t pos);
    
    const char * t_rank_tree_dump(void);
    int t_rank_tree_error_pos(void);
    
    rt_t m_rank_tree;
};

#endif
