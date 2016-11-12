#include "XComputerTest.hpp"

TEST_F(XComputerTest, add_int) {
    ASSERT_TRUE(calc("1 + 1"));

    ASSERT_RESULT_INT_EQ(2);
}

TEST_F(XComputerTest, add_int_float) {
    ASSERT_TRUE(calc("1 + 1.0"));

    ASSERT_RESULT_FLOAT_EQ(2.0);
}

TEST_F(XComputerTest, add_int_str) {
    ASSERT_TRUE(calc("1 + '2'"));

    ASSERT_RESULT_STR_EQ("12");
}

TEST_F(XComputerTest, add_float) {
    ASSERT_TRUE(calc("1.0 + 1.0"));

    ASSERT_RESULT_FLOAT_EQ(2.0);
}

TEST_F(XComputerTest, add_str) {
    ASSERT_TRUE(calc("'1' + '2'"));

    ASSERT_RESULT_STR_EQ("12");
}

TEST_F(XComputerTest, add_str_int) {
    ASSERT_TRUE(calc("'1' + 2"));

    ASSERT_RESULT_STR_EQ("12");
}
