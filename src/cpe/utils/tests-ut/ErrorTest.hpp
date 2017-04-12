#ifndef CPE_DR_TEST_WITH_ERRORTEST_H
#define CPE_DR_TEST_WITH_ERRORTEST_H
#include "gtest/gtest.h"
#include "cpe/utils/error.h"
#include "cpe/utils/error_list.h"

class ErrorTest : public ::testing::Test {
public:
    ErrorTest();

    virtual void SetUp();
    virtual void TearDown();

    error_list_t m_el_1;
    error_list_t m_el_2;
};

#endif

