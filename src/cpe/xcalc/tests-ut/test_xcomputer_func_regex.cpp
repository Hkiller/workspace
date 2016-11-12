#include "XComputerTest.hpp"

TEST_F(XComputerTest, regex_basic) {
    t_em_set_print();
    ASSERT_TRUE(calc("regex('12cd', '^(\\d*).*$')"));
    ASSERT_RESULT_STR_EQ("12");
}
