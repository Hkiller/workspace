#include "cpe/pal/pal_stdlib.h"
#include "RankTreeTest.hpp"

class ContextSortTest : public RankTreeTest {
    virtual void SetUp() {
        RankTreeTest::SetUp();
        t_rank_tree_create(100);
    }
};

TEST_F(ContextSortTest, random) {
    uint32_t values[100];
    for(uint32_t i = 0; i < CPE_ARRAY_SIZE(values); ++i) {
        values[i] = i + 1;
    }

    for(uint32_t i = 0; i < CPE_ARRAY_SIZE(values); ++i) {
        uint32_t left_count = CPE_ARRAY_SIZE(values) - i - 1;
        if (left_count > 1) {
            uint32_t swap_pos = i + ((uint32_t)random()) % left_count;
            uint32_t t = values[i];
            values[i] = values[swap_pos];
            values[swap_pos] = t;
        }
    }
    
    t_rank_tree_install(values, CPE_ARRAY_SIZE(values));

    char expect[512];
    int n = 0;
    for(uint32_t i = 0; i < CPE_ARRAY_SIZE(values); ++i) {
        if (n) {
            n += snprintf(expect + n, sizeof(expect) - n, " ");
        }

        n += snprintf(expect + n, sizeof(expect) - n, "%d", CPE_ARRAY_SIZE(values) - i);
    }
    
    EXPECT_STREQ(expect, this->values());
}

TEST_F(ContextSortTest, rank_1) {
    uint32_t values[] = { 9827, 400, 400, 2540, 18187, 400, 400, 400 };

    t_rank_tree_install(values, CPE_ARRAY_SIZE(values));
    
    EXPECT_STREQ(
        "18187 9827 2540 400 400 400 400 400" , this->values());
}
