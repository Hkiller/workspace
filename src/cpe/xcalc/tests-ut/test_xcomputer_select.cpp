#include "XComputerTest.hpp"

TEST_F(XComputerTest, select_basic) {
    ASSERT_TRUE(calc("1 ? 1 : 'a', 2: 'b'"));
    ASSERT_RESULT_STR_EQ("a");
}

TEST_F(XComputerTest, select_default) {
    t_em_set_print();
    ASSERT_TRUE(calc("3 ? 1 : 'a', 2: 'b', 'c'"));
    ASSERT_RESULT_STR_EQ("c");
}

TEST_F(XComputerTest, select_not_exist) {
    ASSERT_FALSE(calc("3 ? 1 : 'a', 2: 'b'"));
}

