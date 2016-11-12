#include "CfgTest.hpp"
#include "../cfg_internal_types.h"

class SeqTest : public CfgTest {
public:
    SeqTest() : m_seq(NULL) {}

    virtual void SetUp() {
        CfgTest::SetUp();

        m_seq = cfg_struct_add_seq(m_root, "a", cfg_replace);
        ASSERT_TRUE(m_seq);
    }

    void fill(int n) {
        while(n > 0) {
            cfg_seq_add_struct(m_seq);
            --n;
        }
    }

    cfg_t m_seq;
};

TEST_F(SeqTest, count) {
    EXPECT_EQ(0, cfg_seq_count(m_seq));
}

TEST_F(SeqTest, add_basic) {
    cfg_t s = cfg_seq_add_struct(m_seq);
    ASSERT_TRUE(s);
    EXPECT_STREQ("a", cfg_name(s));

    EXPECT_EQ(CPE_CFG_TYPE_STRUCT, cfg_type(s));
    EXPECT_TRUE(m_seq == cfg_parent(s));

    EXPECT_EQ(1, cfg_seq_count(m_seq));
}

TEST_F(SeqTest, add_order) {
    cfg_t s1 = cfg_seq_add_struct(m_seq);
    ASSERT_TRUE(s1);

    cfg_t s2 = cfg_seq_add_struct(m_seq);
    ASSERT_TRUE(s2);

    EXPECT_EQ(2, cfg_seq_count(m_seq));

    EXPECT_TRUE(s1 == cfg_seq_at(m_seq, 0));
    EXPECT_TRUE(s2 == cfg_seq_at(m_seq, 1));
}

TEST_F(SeqTest, add_to_next_trunk) {
    fill(CPE_CFG_SEQ_BLOCK_ITEM_COUNT);
    cfg_t s1 = cfg_seq_add_struct(m_seq);
    ASSERT_TRUE(s1);

    cfg_t s2 = cfg_seq_add_struct(m_seq);
    ASSERT_TRUE(s2);

    EXPECT_EQ(CPE_CFG_SEQ_BLOCK_ITEM_COUNT + 2, cfg_seq_count(m_seq));

    EXPECT_TRUE(s1 == cfg_seq_at(m_seq, CPE_CFG_SEQ_BLOCK_ITEM_COUNT + 0));
    EXPECT_TRUE(s2 == cfg_seq_at(m_seq, CPE_CFG_SEQ_BLOCK_ITEM_COUNT + 1));
}

TEST_F(SeqTest, add_not_from_seq) {
    cfg_t not_seq = cfg_struct_add_int8(m_root, "aa", 4, cfg_replace);
    EXPECT_FALSE(cfg_seq_add_struct(not_seq));
}


TEST_F(SeqTest, at_from_empty) {
    EXPECT_TRUE(NULL == cfg_seq_at(m_seq, 0));
}

TEST_F(SeqTest, at_pos_negative) {
    cfg_seq_add_struct(m_seq);

    EXPECT_FALSE(cfg_seq_at(m_seq, -2));
}

TEST_F(SeqTest, at_pos_overflow) {
    cfg_seq_add_struct(m_seq);

    EXPECT_FALSE(cfg_seq_at(m_seq, 1));
}

TEST_F(SeqTest, at_basic) {
    cfg_t s = cfg_seq_add_struct(m_seq);

    EXPECT_TRUE(s == cfg_seq_at(m_seq, 0));
}

TEST_F(SeqTest, remove_last) {
    cfg_t i0 = cfg_seq_add_struct(m_seq);
    cfg_t i1 = cfg_seq_add_struct(m_seq);
    cfg_t i2 = cfg_seq_add_struct(m_seq);

    cfg_free(i2);

    EXPECT_EQ(2, cfg_seq_count(m_seq));
    EXPECT_EQ(i0, cfg_seq_at(m_seq, 0));
    EXPECT_EQ(i1, cfg_seq_at(m_seq, 1));
}

TEST_F(SeqTest, remove_middle) {
    cfg_t i0 = cfg_seq_add_struct(m_seq);
    cfg_t i1 = cfg_seq_add_struct(m_seq);
    cfg_t i2 = cfg_seq_add_struct(m_seq);

    cfg_free(i1);

    EXPECT_EQ(2, cfg_seq_count(m_seq));
    EXPECT_EQ(i0, cfg_seq_at(m_seq, 0));
    EXPECT_EQ(i2, cfg_seq_at(m_seq, 1));
}

TEST_F(SeqTest, remove_first) {
    cfg_t i0 = cfg_seq_add_struct(m_seq);
    cfg_t i1 = cfg_seq_add_struct(m_seq);
    cfg_t i2 = cfg_seq_add_struct(m_seq);

    cfg_free(i0);

    EXPECT_EQ(2, cfg_seq_count(m_seq));
    EXPECT_EQ(i1, cfg_seq_at(m_seq, 0));
    EXPECT_EQ(i2, cfg_seq_at(m_seq, 1));
}

TEST_F(SeqTest, remove_trunk_last_no_left) {
    fill(CPE_CFG_SEQ_BLOCK_ITEM_COUNT - 2);
    cfg_t l0 = cfg_seq_add_struct(m_seq);
    cfg_t l1 = cfg_seq_add_struct(m_seq);

    cfg_free(l1);

    EXPECT_EQ(CPE_CFG_SEQ_BLOCK_ITEM_COUNT - 1, cfg_seq_count(m_seq));
    EXPECT_EQ(l0, cfg_seq_at(m_seq, CPE_CFG_SEQ_BLOCK_ITEM_COUNT - 2));
}

TEST_F(SeqTest, remove_trunk_last_with_one_left) {
    fill(CPE_CFG_SEQ_BLOCK_ITEM_COUNT - 1);
    cfg_t l0 = cfg_seq_add_struct(m_seq);
    cfg_t l1 = cfg_seq_add_struct(m_seq);

    cfg_free(l0);

    EXPECT_EQ(CPE_CFG_SEQ_BLOCK_ITEM_COUNT, cfg_seq_count(m_seq));
    EXPECT_EQ(l1, cfg_seq_at(m_seq, CPE_CFG_SEQ_BLOCK_ITEM_COUNT - 1));
}

TEST_F(SeqTest, remove_trunk_last_with_more_left) {
    fill(CPE_CFG_SEQ_BLOCK_ITEM_COUNT - 1);
    cfg_t l0 = cfg_seq_add_struct(m_seq);
    cfg_t l1 = cfg_seq_add_struct(m_seq);
    cfg_t l2 = cfg_seq_add_struct(m_seq);

    cfg_free(l0);

    EXPECT_EQ(CPE_CFG_SEQ_BLOCK_ITEM_COUNT + 1, cfg_seq_count(m_seq));
    EXPECT_EQ(l1, cfg_seq_at(m_seq, CPE_CFG_SEQ_BLOCK_ITEM_COUNT - 1));
    EXPECT_EQ(l2, cfg_seq_at(m_seq, CPE_CFG_SEQ_BLOCK_ITEM_COUNT));
}

TEST_F(SeqTest, it_empty) {
    cfg_it_t it;
    cfg_it_init(&it, m_seq);

    ASSERT_TRUE(cfg_it_next(&it) == NULL);
}

TEST_F(SeqTest, it_basic) {
    cfg_seq_add_int32(m_seq, 1);
    cfg_seq_add_int32(m_seq, 2);

    cfg_it_t it;
    cfg_it_init(&it, m_seq);

    cfg_t cfg_a = cfg_it_next(&it);
    ASSERT_TRUE(cfg_a);
    ASSERT_EQ(1, cfg_as_int32(cfg_a, -1));

    cfg_t cfg_b = cfg_it_next(&it);
    ASSERT_TRUE(cfg_b);
    ASSERT_EQ(2, cfg_as_int32(cfg_b, -1));

    ASSERT_TRUE(cfg_it_next(&it) == NULL);
}

TEST_F(SeqTest, it_init_cfg_null) {
    cfg_it_t it;
    cfg_it_init(&it, NULL);

    ASSERT_TRUE(cfg_it_next(&it) == NULL);
}

