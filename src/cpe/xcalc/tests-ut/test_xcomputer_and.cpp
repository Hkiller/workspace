#include "XComputerTest.hpp"

TEST_F(XComputerTest, and_int) {
    ASSERT_TRUE(calc("1 && 2"));
    ASSERT_RESULT_INT_EQ(1);

    ASSERT_TRUE(calc("0 && 0"));
    ASSERT_RESULT_INT_EQ(0);

    ASSERT_TRUE(calc("0 && 1"));
    ASSERT_RESULT_INT_EQ(0);

    ASSERT_TRUE(calc("1 && 0"));
    ASSERT_RESULT_INT_EQ(0);
}

TEST_F(XComputerTest, and_int_float) {
    ASSERT_TRUE(calc("1 && 2.0"));
    ASSERT_RESULT_INT_EQ(1);

    ASSERT_TRUE(calc("0 && 0.0"));
    ASSERT_RESULT_INT_EQ(0);

    ASSERT_TRUE(calc("0 && 1.0"));
    ASSERT_RESULT_INT_EQ(0);

    ASSERT_TRUE(calc("1 && 0.0"));
    ASSERT_RESULT_INT_EQ(0);
}

TEST_F(XComputerTest, and_int_str) {
    ASSERT_TRUE(calc("1 && 'a'"));
    ASSERT_RESULT_INT_EQ(1);

    ASSERT_TRUE(calc("0 && ''"));
    ASSERT_RESULT_INT_EQ(0);

    ASSERT_TRUE(calc("0 && 'a'"));
    ASSERT_RESULT_INT_EQ(0);

    ASSERT_TRUE(calc("1 && ''"));
    ASSERT_RESULT_INT_EQ(0);
}
