#include "XComputerTest.hpp"

TEST_F(XComputerTest, strlen_basic) {
    ASSERT_TRUE(calc("strlen('abc')"));
    ASSERT_RESULT_INT_EQ(3);
}
