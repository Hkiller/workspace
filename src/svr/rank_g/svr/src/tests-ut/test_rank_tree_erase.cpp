#include "cpe/pal/pal_stdlib.h"
#include "RankTreeTest.hpp"

class ContextEraseTest : public RankTreeTest {
    virtual void SetUp() {
        RankTreeTest::SetUp();
        t_rank_tree_create(100);
    }
};

TEST_F(ContextEraseTest, random_no_duplic) {
    uint32_t values[100];
    for(uint32_t i = 0; i < CPE_ARRAY_SIZE(values); ++i) {
        values[i] = i + 1;
    }
    t_rank_tree_install(values, CPE_ARRAY_SIZE(values));
    ASSERT_TRUE(t_rank_tree_error_pos() < 0);

    uint32_t count = CPE_ARRAY_SIZE(values);
    while(count > 0) {
        uint32_t remove_idx = ((uint32_t)random()) % count;
        rt_node_t remove = get_at(remove_idx);
        ASSERT_TRUE(remove);
        uint32_t remove_value = remove->m_value;

        t_rank_tree_erase(remove);

        if (t_rank_tree_error_pos() >= 0) {
            ASSERT_EQ(-1, t_rank_tree_error_pos())
                << "left-count=" << count
                << ", remove-value=" << remove_value;
        }
        
        --count;
    }
}
