#include "XComputerTest.hpp"

TEST_F(XComputerTest, basic_int) {
    ASSERT_TRUE(calc("1"));

    ASSERT_RESULT_INT_EQ(1);
}

TEST_F(XComputerTest, basic_int_negative) {
    ASSERT_TRUE(calc("-1"));

    ASSERT_RESULT_INT_EQ(-1);
}

TEST_F(XComputerTest, basic_float) {
    ASSERT_TRUE(calc("1.3"));

    ASSERT_RESULT_FLOAT_EQ(1.3);
}

TEST_F(XComputerTest, basic_float_negative) {
    ASSERT_TRUE(calc("-1.3"));

    ASSERT_RESULT_FLOAT_EQ(-1.3);
}

TEST_F(XComputerTest, basic_string) {
    ASSERT_TRUE(calc("'abc'"));

    ASSERT_RESULT_STR_EQ("abc");
}

TEST_F(XComputerTest, basic_string_2) {
    ASSERT_TRUE(calc("abc"));

    ASSERT_RESULT_STR_EQ("abc");
}

TEST_F(XComputerTest, basic_string_3) {
    ASSERT_TRUE(calc("\"abc\""));

    ASSERT_RESULT_STR_EQ("abc");
}

TEST_F(XComputerTest, basic_string_spectial) {
    ASSERT_TRUE(calc("abc-"));

    ASSERT_RESULT_STR_EQ("abc-");
}
