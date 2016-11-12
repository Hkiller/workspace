#include "PriorityQueueTest.hpp"

TEST_F(PriorityQueueTest, basic) {
    int32_t a[]={1, 9, 7, 8, 5, 4, 3, 2, 1, 100, 50, 17};
    for(uint16_t i = 0; i < CPE_ARRAY_SIZE(a); ++i) {
        insert(a[i]);
    }

    EXPECT_EQ(CPE_ARRAY_SIZE(a), count());
    EXPECT_STREQ("1 1 2 3 4 5 7 8 9 17 50 100", dump_dequeue());
}

TEST_F(PriorityQueueTest, basic2) {
    int32_t a[]={1, 9, 7, 8, 5, 4, 3, 2, 1, 100, 50, 17};
    for(uint16_t i = CPE_ARRAY_SIZE(a); i > 0; i--) {
        insert(a[i - 1]);
    }

    EXPECT_EQ(CPE_ARRAY_SIZE(a), count());
    EXPECT_STREQ("1 1 2 3 4 5 7 8 9 17 50 100", dump_dequeue());
}

TEST_F(PriorityQueueTest, basic3) {
    insert(3);
    insert(5);
    insert(1);

    EXPECT_EQ(3, count());
    EXPECT_STREQ("1 3 5", dump_dequeue());
}
