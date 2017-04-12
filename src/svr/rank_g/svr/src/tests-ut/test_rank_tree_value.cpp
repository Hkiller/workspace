#include "RankTreeTest.hpp"

class ContextValueTest : public RankTreeTest {
    virtual void SetUp() {
        RankTreeTest::SetUp();
        t_rank_tree_create(100);
    }
};

TEST_F(ContextValueTest, basic) {
    uint32_t values[] = { 1, 2, 3, 3, 4 };
    t_rank_tree_install(values, CPE_ARRAY_SIZE(values));

    EXPECT_STREQ("3 4", range(3));
}

TEST_F(ContextValueTest, range) {
    uint32_t values[] = { 7, 7, 7, 6, 6, 6, 5, 5, 5 };
    t_rank_tree_install(values, CPE_ARRAY_SIZE(values));

    EXPECT_STREQ("1 2 3", range(7));
    EXPECT_STREQ("4 5 6", range(6));
    EXPECT_STREQ("7 8 9", range(5));
}

TEST_F(ContextValueTest, find_min) {
    uint32_t values[] = { 7, 7, 7, 6, 6, 6, 5, 5, 5 };
    t_rank_tree_install(values, CPE_ARRAY_SIZE(values));

    rt_node_t r = find_min(7);
    ASSERT_TRUE(r);
    ASSERT_EQ(1, r->m_record_id);

    r = find_min(6);
    ASSERT_TRUE(r);
    ASSERT_EQ(4, r->m_record_id);

    r = find_min(5);
    ASSERT_TRUE(r);
    ASSERT_EQ(7, r->m_record_id);
}

TEST_F(ContextValueTest, find_max) {
    uint32_t values[] = { 7, 7, 7, 6, 6, 6, 5, 5, 5 };
    t_rank_tree_install(values, CPE_ARRAY_SIZE(values));

    rt_node_t r = find_max(7);
    ASSERT_TRUE(r);
    ASSERT_EQ(4, r->m_record_id);

    r = find_max(6);
    ASSERT_TRUE(r);
    ASSERT_EQ(7, r->m_record_id);

    r = find_max(5);
    ASSERT_TRUE(r == NULL);
}

