#include "XComputerTest.hpp"

TEST_F(XComputerTest, div_int) {
    ASSERT_TRUE(calc("3 / 2"));

    ASSERT_RESULT_INT_EQ(1);
}

TEST_F(XComputerTest, div_int_zero) {
    ASSERT_FALSE(calc("3 / 0"));
}

TEST_F(XComputerTest, div_int_float) {
    ASSERT_TRUE(calc("3 / 2.0"));

    ASSERT_RESULT_FLOAT_EQ(1.5);
}

TEST_F(XComputerTest, div_float_int) {
    ASSERT_TRUE(calc("3.0 / 2"));

    ASSERT_RESULT_FLOAT_EQ(1.5);
}

TEST_F(XComputerTest, div_float_float) {
    ASSERT_TRUE(calc("3.0 / 2.0"));

    ASSERT_RESULT_FLOAT_EQ(1.5);
}

TEST_F(XComputerTest, div_float_zero) {
    ASSERT_FALSE(calc("3 / 0.0f"));
}

TEST_F(XComputerTest, div_str) {
    ASSERT_FALSE(calc("'1' / '2'"));
}


