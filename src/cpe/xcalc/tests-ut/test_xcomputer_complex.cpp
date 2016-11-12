#include "XComputerTest.hpp"

TEST_F(XComputerTest, complex_1) {
    ASSERT_TRUE(calc("@a + (@b ? 0 : 50, 1: 51)", "a=23, b=1"));

    ASSERT_RESULT_INT_EQ(74);
}

TEST_F(XComputerTest, complex_ask_brack) {
    ASSERT_TRUE(calc("@a ? 0 : (10 + 50), (10 - 50)", "a=1"));
    ASSERT_RESULT_INT_EQ(-40);
}
