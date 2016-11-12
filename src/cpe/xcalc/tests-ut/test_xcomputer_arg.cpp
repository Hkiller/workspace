#include "XComputerTest.hpp"

TEST_F(XComputerTest, arg_basic_int) {
    ASSERT_TRUE(calc("@a", "a=1"));

    ASSERT_RESULT_INT_EQ(1);
}

TEST_F(XComputerTest, arg_basic_multi_char) {
    ASSERT_TRUE(calc("@abc", "abc=1"));

    ASSERT_RESULT_INT_EQ(1);
}

TEST_F(XComputerTest, arg_ignore_space_before) {
    ASSERT_TRUE(calc(" \t@abc", "abc=1"));
    ASSERT_RESULT_INT_EQ(1);
}

TEST_F(XComputerTest, arg_ignore_space_after) {
    ASSERT_TRUE(calc("@abc\t", "abc=1"));
    ASSERT_RESULT_INT_EQ(1);

    ASSERT_TRUE(calc("@abc ", "abc=1"));
    ASSERT_RESULT_INT_EQ(1);
}

TEST_F(XComputerTest, arg_spectial) {
    ASSERT_TRUE(calc("@a-_[]{}@", "a-_[]{}@=1"));

    ASSERT_RESULT_INT_EQ(1);
}

TEST_F(XComputerTest, arg_basic_float) {
    ASSERT_TRUE(calc("@a", "a=2.1"));

    ASSERT_RESULT_FLOAT_EQ(2.1);
}

TEST_F(XComputerTest, arg_basic_str) {
    ASSERT_TRUE(calc("@a", "a=bcd"));

    ASSERT_RESULT_STR_EQ("bcd");
}

TEST_F(XComputerTest, arg_not_exist) {
    ASSERT_FALSE(calc("@a"));
}

TEST_F(XComputerTest, arg_eq_str) {
    ASSERT_TRUE(calc("@a==b", "a=bcd"));
    ASSERT_RESULT_INT_EQ(0);

    ASSERT_TRUE(calc("@a==abc", "a=abc"));
    ASSERT_RESULT_INT_EQ(1);
}
