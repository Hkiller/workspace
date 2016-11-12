#ifndef CPE_DR_TEST_COMPAIRETEST_H
#define CPE_DR_TEST_COMPAIRETEST_H
#include "gtest/gtest.h"
#include "cpe/dr/dr_ctypes_op.h"

class CTypeCompaireTest : public ::testing::Test {
public:
    int comp(int lType, const char * lValue, int rType, const char * rValue);
};

#define CTYPE_ASSERT_EQ(__value, __l_type, __r_type) \
    EXPECT_EQ(0, comp(__l_type, __value, __r_type, __value)); \
    EXPECT_EQ(0, comp(__r_type, __value, __l_type, __value))

#define CTYPE_ASSERT_LT(__l_value, __l_type, __r_value, __r_type)   \
    EXPECT_LT(comp(__l_type, __l_value, __r_type, __r_value), 0);   \
    EXPECT_GT(comp(__r_type, __r_value, __l_type, __l_value), 0)

#endif
