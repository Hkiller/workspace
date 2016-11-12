#include "XComputerTest.hpp"

TEST_F(XComputerTest, sub_int) {
    ASSERT_TRUE(calc("1 - 2"));

    ASSERT_RESULT_INT_EQ(-1);
}

TEST_F(XComputerTest, sub_int_float) {
    ASSERT_TRUE(calc("1 - 2.0"));

    ASSERT_RESULT_FLOAT_EQ(-1.0);
}

TEST_F(XComputerTest, sub_float) {
    ASSERT_TRUE(calc("1.0 - 2.0"));

    ASSERT_RESULT_FLOAT_EQ(-1.0);
}

TEST_F(XComputerTest, sub_str) {
    ASSERT_FALSE(calc("'1' - '2'"));
}

