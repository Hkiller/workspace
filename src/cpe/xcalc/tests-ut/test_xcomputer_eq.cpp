#include "XComputerTest.hpp"

TEST_F(XComputerTest, eq_int) {
    ASSERT_TRUE(calc("1 == 2"));
    ASSERT_RESULT_INT_EQ(0);

    ASSERT_TRUE(calc("1 == 1"));
    ASSERT_RESULT_INT_EQ(1);
}

TEST_F(XComputerTest, eq_int_float) {
    ASSERT_TRUE(calc("1 == 2.0"));
    ASSERT_RESULT_INT_EQ(0);

    ASSERT_TRUE(calc("1 == 1.0"));
    ASSERT_RESULT_INT_EQ(1);
}

TEST_F(XComputerTest, eq_int_str) {
    ASSERT_TRUE(calc("1 == '2'"));
    ASSERT_RESULT_INT_EQ(0);

    ASSERT_TRUE(calc("1 == '1'"));
    ASSERT_RESULT_INT_EQ(1);
}

TEST_F(XComputerTest, eq_float) {
    ASSERT_TRUE(calc("1 == 2"));
    ASSERT_RESULT_INT_EQ(0);

    ASSERT_TRUE(calc("1 == 1"));
    ASSERT_RESULT_INT_EQ(1);
}

TEST_F(XComputerTest, eq_float_int) {
    ASSERT_TRUE(calc("1.0 == 2"));
    ASSERT_RESULT_INT_EQ(0);

    ASSERT_TRUE(calc("1.0 == 1"));
    ASSERT_RESULT_INT_EQ(1);
}

TEST_F(XComputerTest, eq_float_str) {
    ASSERT_TRUE(calc("1.0 == '2'"));
    ASSERT_RESULT_INT_EQ(0);

    ASSERT_TRUE(calc("1.0 == '1.0'"));
    ASSERT_RESULT_INT_EQ(1);
}
