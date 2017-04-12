#include "cpepp/utils/Random.hpp"
#include "WeightSelectorTest.hpp"

TEST_F(WeightSelectorTest, empty) {
    EXPECT_EQ(-1, select());
}

TEST_F(WeightSelectorTest, select_basic) {
    add(3);
    add(3);

    t_random_expect_gen_with_arg(6, 0);

    EXPECT_EQ(0, select());
}

TEST_F(WeightSelectorTest, select_one_basic) {
    add(3);

    t_random_expect_gen_with_arg(3, 3);

    EXPECT_EQ(-1, select());
}

TEST_F(WeightSelectorTest, select_random_overflow) {
    add(3);
    add(3);

    t_random_expect_gen_with_arg(6, 0);
    EXPECT_EQ(0, select());

    t_random_expect_gen_with_arg(6, 1);
    EXPECT_EQ(0, select());

    t_random_expect_gen_with_arg(6, 2);
    EXPECT_EQ(0, select());

    t_random_expect_gen_with_arg(6, 3);
    EXPECT_EQ(1, select());

    t_random_expect_gen_with_arg(6, 4);
    EXPECT_EQ(1, select());

    t_random_expect_gen_with_arg(6, 5);
    EXPECT_EQ(1, select());
}

TEST_F(WeightSelectorTest, select_zero) {
    add(0);

    t_random_expect_gen_with_arg(1, 0);

    EXPECT_EQ(0, select());
}

TEST_F(WeightSelectorTest, select_zero_multi) {
    add(0);
    add(0);

    t_random_expect_gen_with_arg(2, 1);

    EXPECT_EQ(1, select());
}

TEST_F(WeightSelectorTest, select_zero_overflow) {
    add(0);
    add(0);

    t_random_expect_gen_with_arg(2, 2);

    EXPECT_EQ(-1, select());
}

TEST_F(WeightSelectorTest, property) {
    uint32_t weits[] = {
        0,
        0,
        25,
        60,
        15,
        0,
        0,
        25,
        60,
        15,
        0,
        0,
        25,
        60,
        15
    };

    int result[sizeof(weits) / sizeof(weits[0])];

    size_t count = sizeof(weits) / sizeof(weits[0]);

    uint32_t total_weight = 0;
    for(size_t i = 0; i < count; ++i) {
        add(weits[i]);
        total_weight += weits[i];
        result[i] = 0;
    }

    size_t repeat_count = 100000;
    for(size_t i = 0; i < repeat_count; ++i) {
        uint32_t r = m_selector.select(Cpe::Utils::Random::dft());
        ASSERT_GE((int)r, 0);
        ASSERT_LT((size_t)r, count);
        result[r]++;
    }

    for(size_t i = 0; i < count; ++i) {
        double expect = ((double)weits[i]) / (double)total_weight;
        double actually = ((double)result[i]) / (double)repeat_count;

        EXPECT_NEAR(expect, actually, 0.02);
    }
}
