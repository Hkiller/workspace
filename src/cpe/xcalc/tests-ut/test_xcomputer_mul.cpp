#include "XComputerTest.hpp"

TEST_F(XComputerTest, mul_int) {
    ASSERT_TRUE(calc("2 * -3"));

    ASSERT_RESULT_INT_EQ(-6);
}

TEST_F(XComputerTest, mul_int_float) {
    ASSERT_TRUE(calc("2 * -3.0"));

    ASSERT_RESULT_FLOAT_EQ(-6.0);
}

TEST_F(XComputerTest, mul_float) {
    ASSERT_TRUE(calc("2.0 * -3.0"));

    ASSERT_RESULT_FLOAT_EQ(-6.0);
}

TEST_F(XComputerTest, mul_str) {
    ASSERT_FALSE(calc("'1' * '2'"));
}

