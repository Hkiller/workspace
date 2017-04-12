#ifndef CPE_XCALC_XCOMPUTER_TEST_H
#define CPE_XCALC_XCOMPUTER_TEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/xcalc/xcalc_computer.h"
#include "../xcalc_token_i.h"

typedef LOKI_TYPELIST_1(
    utils::testenv::with_em) XComputerTestBase;

class XComputerTest : public testenv::fixture<XComputerTestBase> {
public:
    virtual void SetUp();
    virtual void TearDown();

    xcomputer_t m_computer;
    xtoken_t m_result;

    bool calc(const char * def, const char * args = NULL);
};

#define ASSERT_RESULT_INT_EQ(__expect)                      \
    ASSERT_TRUE(m_result != NULL);                          \
    ASSERT_EQ((uint32_t)XTOKEN_NUM_INT, m_result->m_type);  \
    ASSERT_EQ(__expect, m_result->m_data.num._int);

#define ASSERT_RESULT_FLOAT_EQ(__expect)                      \
    ASSERT_TRUE(m_result != NULL);                          \
    ASSERT_EQ((uint32_t)XTOKEN_NUM_FLOAT, m_result->m_type);  \
    ASSERT_EQ(__expect, m_result->m_data.num._double);

#define ASSERT_RESULT_STR_EQ(__expect)                      \
    ASSERT_TRUE(m_result != NULL);                          \
    ASSERT_EQ((uint32_t)XTOKEN_STRING, m_result->m_type);   \
    ASSERT_STREQ(__expect, m_result->m_data.str._string);

#endif
