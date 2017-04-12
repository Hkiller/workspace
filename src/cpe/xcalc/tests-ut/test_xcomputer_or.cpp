#include "XComputerTest.hpp"

TEST_F(XComputerTest, or_int) {
    ASSERT_TRUE(calc("1 || 0"));
    ASSERT_RESULT_INT_EQ(1);

    ASSERT_TRUE(calc("1 || 1"));
    ASSERT_RESULT_INT_EQ(1);

    ASSERT_TRUE(calc("0 || 1"));
    ASSERT_RESULT_INT_EQ(1);

    ASSERT_TRUE(calc("0 || 0"));
    ASSERT_RESULT_INT_EQ(0);
}

TEST_F(XComputerTest, or_int_float) {
    ASSERT_TRUE(calc("0 || 2.0"));
    ASSERT_RESULT_INT_EQ(1);

    ASSERT_TRUE(calc("0 || 0.0"));
    ASSERT_RESULT_INT_EQ(0);

    ASSERT_TRUE(calc("0 || 1.0"));
    ASSERT_RESULT_INT_EQ(1);

    ASSERT_TRUE(calc("0 || 0.0"));
    ASSERT_RESULT_INT_EQ(0);
}

TEST_F(XComputerTest, or_int_str) {
    ASSERT_TRUE(calc("0 || 'a'"));
    ASSERT_RESULT_INT_EQ(1);

    ASSERT_TRUE(calc("0 || ''"));
    ASSERT_RESULT_INT_EQ(0);

    ASSERT_TRUE(calc("0 || 'a'"));
    ASSERT_RESULT_INT_EQ(1);

    ASSERT_TRUE(calc("0 || ''"));
    ASSERT_RESULT_INT_EQ(0);
}
