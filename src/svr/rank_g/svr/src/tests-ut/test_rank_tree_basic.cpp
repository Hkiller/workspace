#include "RankTreeTest.hpp"

class ContextBasicTest : public RankTreeTest {
    virtual void SetUp() {
        RankTreeTest::SetUp();
        t_rank_tree_create(100);
    }
};

TEST_F(ContextBasicTest, empty) {
    EXPECT_EQ((uint32_t)0, rt_size(m_rank_tree));
    EXPECT_EQ((uint32_t)100, rt_capacity(m_rank_tree));

    EXPECT_TRUE(rt_first(m_rank_tree) == NULL);
    EXPECT_TRUE(rt_last(m_rank_tree) == NULL);
}

TEST_F(ContextBasicTest, sort) {
    uint32_t values[] = { 90, 91, 92, 93, 94 };
    t_rank_tree_install(values, CPE_ARRAY_SIZE(values));

    EXPECT_STREQ("5 4 3 2 1", range());
}

TEST_F(ContextBasicTest, insert_first) {
    rt_node_t node = rt_insert(m_rank_tree, 5, 1);
    ASSERT_TRUE(node != NULL);

    EXPECT_TRUE(rt_first(m_rank_tree) == node);
    EXPECT_TRUE(rt_last(m_rank_tree) == node);

    EXPECT_EQ((uint32_t)1, rt_size(m_rank_tree));
}

TEST_F(ContextBasicTest, insert_multi) {
    rt_node_t node1 = rt_insert(m_rank_tree, 5, 1);
    ASSERT_TRUE(node1 != NULL);
    EXPECT_EQ((uint32_t)5, rt_node_value(node1));

    rt_node_t node2 = rt_insert(m_rank_tree, 7, 2);
    ASSERT_TRUE(node2 != NULL);
    EXPECT_EQ((uint32_t)7, rt_node_value(node2));

    rt_node_t node3 = rt_insert(m_rank_tree, 2, 3);
    ASSERT_TRUE(node3 != NULL);
    EXPECT_EQ((uint32_t)2, rt_node_value(node3));

    EXPECT_TRUE(rt_first(m_rank_tree) == node2);
    EXPECT_TRUE(rt_last(m_rank_tree) == node3);

    EXPECT_EQ((uint32_t)3, rt_size(m_rank_tree));
}

TEST_F(ContextBasicTest, move) {
    rt_node_t node1 = rt_insert(m_rank_tree, 5, 1);
    ASSERT_TRUE(node1 != NULL);

    rt_node_t node2 = rt_insert(m_rank_tree, 7, 2);
    ASSERT_TRUE(node2 != NULL);

    rt_node_t node3 = rt_insert(m_rank_tree, 2, 3);
    ASSERT_TRUE(node3 != NULL);

    EXPECT_TRUE(rt_next(m_rank_tree, node2) == node1);
    EXPECT_TRUE(rt_next(m_rank_tree, node1) == node3);
    EXPECT_TRUE(rt_next(m_rank_tree, node3) == NULL);

    EXPECT_TRUE(rt_pre(m_rank_tree, node3) == node1);
    EXPECT_TRUE(rt_pre(m_rank_tree, node1) == node2);
    EXPECT_TRUE(rt_pre(m_rank_tree, node2) == NULL);
}

TEST_F(ContextBasicTest, erase_first) {
    rt_node_t node1 = rt_insert(m_rank_tree, 5, 1);
    ASSERT_TRUE(node1 != NULL);
    rt_node_t node2 = rt_insert(m_rank_tree, 7, 2);
    ASSERT_TRUE(node2 != NULL);
    rt_node_t node3 = rt_insert(m_rank_tree, 2, 3);
    ASSERT_TRUE(node3 != NULL);

    rt_erase(m_rank_tree, node2);

    EXPECT_TRUE(rt_next(m_rank_tree, node1) == node3);
    EXPECT_TRUE(rt_next(m_rank_tree, node3) == NULL);

    EXPECT_TRUE(rt_first(m_rank_tree) == node1);
    EXPECT_TRUE(rt_last(m_rank_tree) == node3);

    EXPECT_EQ((uint32_t)2, rt_size(m_rank_tree));

    EXPECT_STREQ("5 2", values());
    EXPECT_STREQ("1 3", range());
}

TEST_F(ContextBasicTest, erase_middle) {
    rt_node_t node1 = rt_insert(m_rank_tree, 5, 1);
    ASSERT_TRUE(node1 != NULL);
    rt_node_t node2 = rt_insert(m_rank_tree, 7, 2);
    ASSERT_TRUE(node2 != NULL);
    rt_node_t node3 = rt_insert(m_rank_tree, 2, 3);
    ASSERT_TRUE(node3 != NULL);

    rt_erase(m_rank_tree, node1);

    EXPECT_TRUE(rt_next(m_rank_tree, node2) == node3);
    EXPECT_TRUE(rt_next(m_rank_tree, node3) == NULL);

    EXPECT_TRUE(rt_first(m_rank_tree) == node2);
    EXPECT_TRUE(rt_last(m_rank_tree) == node3);

    EXPECT_EQ((uint32_t)2, rt_size(m_rank_tree));

    EXPECT_STREQ("7 2", values());
    EXPECT_STREQ("2 3", range());
}

TEST_F(ContextBasicTest, erase_last) {
    rt_node_t node1 = rt_insert(m_rank_tree, 5, 1);
    ASSERT_TRUE(node1 != NULL);
    rt_node_t node2 = rt_insert(m_rank_tree, 7, 2);
    ASSERT_TRUE(node2 != NULL);
    rt_node_t node3 = rt_insert(m_rank_tree, 2, 3);
    ASSERT_TRUE(node3 != NULL);

    rt_erase(m_rank_tree, node3);

    EXPECT_TRUE(rt_next(m_rank_tree, node2) == node1);
    EXPECT_TRUE(rt_next(m_rank_tree, node1) == NULL);

    EXPECT_TRUE(rt_first(m_rank_tree) == node2);
    EXPECT_TRUE(rt_last(m_rank_tree) == node1);

    EXPECT_EQ((uint32_t)2, rt_size(m_rank_tree));
}

TEST_F(ContextBasicTest, visit_begin_to_end) {
    for(uint8_t i = 0; i < 100; ++i) {
        rt_node_t node1 = rt_insert(m_rank_tree, 100 + i, i + 1);
        ASSERT_TRUE(node1 != NULL);
    }

    rt_node_t node = NULL;
    for(uint8_t i = 0; i < 100; ++i) {
        if (node == NULL) {
            node = rt_first(m_rank_tree);
        }
        else {
            node = rt_next(m_rank_tree, node);
        }

        ASSERT_TRUE(node != NULL);
        EXPECT_EQ(200 - (i + 1), node->m_value);
        EXPECT_EQ(100 - i, node->m_record_id);
    }
}

TEST_F(ContextBasicTest, visit_end_to_begin) {
    for(uint8_t i = 0; i < 100; ++i) {
        rt_node_t node1 = rt_insert(m_rank_tree, 100 + i, i + 1);
        ASSERT_TRUE(node1 != NULL);
    }

    rt_node_t node = NULL;
    for(uint8_t i = 0; i < 100; ++i) {
        if (node == NULL) {
            node = rt_last(m_rank_tree);
        }
        else {
            node = rt_pre(m_rank_tree, node);
        }

        ASSERT_TRUE(node != NULL);
        EXPECT_EQ(100 + i, node->m_value);
        EXPECT_EQ(i + 1, node->m_record_id);
    }
}
