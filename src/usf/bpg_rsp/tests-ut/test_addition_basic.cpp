#include "cpe/dr/dr_metalib_manage.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_rsp/bpg_rsp_addition.h"
#include "BpgTest.hpp"

class AdditionTest : public BpgTest {
public:
    AdditionTest() : m_context(NULL) {
    }

    virtual void SetUp() {
        BpgTest::SetUp();

        m_context = t_logic_context_create();
        ASSERT_TRUE(m_context);
    }

    logic_context_t m_context;
};

TEST_F(AdditionTest, empty_count) {
    EXPECT_EQ((int16_t)0, bpg_rsp_addition_data_count(m_context));
}

TEST_F(AdditionTest, add_basic) {
    EXPECT_EQ(0, bpg_rsp_addition_data_add(m_context, 4));
    EXPECT_EQ(0, bpg_rsp_addition_data_add(m_context, 3));
    EXPECT_EQ(0, bpg_rsp_addition_data_add(m_context, 2));

    ASSERT_EQ((int16_t)3, bpg_rsp_addition_data_count(m_context));
    EXPECT_EQ(2, bpg_rsp_addition_data_at(m_context, 0));
    EXPECT_EQ(3, bpg_rsp_addition_data_at(m_context, 1));
    EXPECT_EQ(4, bpg_rsp_addition_data_at(m_context, 2));
}

TEST_F(AdditionTest, add_duplicate) {
    EXPECT_EQ(0, bpg_rsp_addition_data_add(m_context, 4));
    EXPECT_EQ(0, bpg_rsp_addition_data_add(m_context, 4));
    EXPECT_EQ(0, bpg_rsp_addition_data_add(m_context, 4));

    ASSERT_EQ((int16_t)1, bpg_rsp_addition_data_count(m_context));
    EXPECT_EQ(4, bpg_rsp_addition_data_at(m_context, 0));
}

TEST_F(AdditionTest, remove_basic) {
    EXPECT_EQ(0, bpg_rsp_addition_data_add(m_context, 4));
    EXPECT_EQ(0, bpg_rsp_addition_data_add(m_context, 3));
    EXPECT_EQ(0, bpg_rsp_addition_data_add(m_context, 2));

    EXPECT_EQ(0, bpg_rsp_addition_data_remove(m_context, 3));

    ASSERT_EQ((int16_t)2, bpg_rsp_addition_data_count(m_context));
    EXPECT_EQ(2, bpg_rsp_addition_data_at(m_context, 0));
    EXPECT_EQ(4, bpg_rsp_addition_data_at(m_context, 1));
}

TEST_F(AdditionTest, remove_not_exist) {
    EXPECT_EQ(0, bpg_rsp_addition_data_add(m_context, 4));
    EXPECT_EQ(0, bpg_rsp_addition_data_add(m_context, 2));

    EXPECT_EQ(-1, bpg_rsp_addition_data_remove(m_context, 3));

    ASSERT_EQ((int16_t)2, bpg_rsp_addition_data_count(m_context));
    EXPECT_EQ(2, bpg_rsp_addition_data_at(m_context, 0));
    EXPECT_EQ(4, bpg_rsp_addition_data_at(m_context, 1));
}
