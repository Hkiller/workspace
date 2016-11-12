#include "RankTreeTest.hpp"

class ContextFindTest : public RankTreeTest {
    virtual void SetUp() {
        RankTreeTest::SetUp();
        t_rank_tree_create(100);
    }
};

TEST_F(ContextFindTest, empty) {
    uint32_t values[] = { 5, 7 };
    t_rank_tree_install(values, CPE_ARRAY_SIZE(values));

    rt_node_t r1 = rt_find_by_rank(m_rank_tree, 0);
    ASSERT_TRUE(r1);

    rt_node_t r2 = rt_find_by_rank(m_rank_tree, 1);
    ASSERT_TRUE(r2);
    
    ASSERT_TRUE(rt_find_by_rank(m_rank_tree, 2) == NULL);

    ASSERT_TRUE(r1);
    EXPECT_EQ((uint32_t)7, rt_node_value(r1));    

    ASSERT_TRUE(r2);
    EXPECT_EQ((uint32_t)5, rt_node_value(r2));    
}

