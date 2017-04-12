#ifndef CPE_DR_DATAJSON_TEST_PRINTTEST_H
#define CPE_DR_DATAJSON_TEST_PRINTTEST_H
#include <string.h>
#include "gtest/gtest.h"
#include "cpe/utils/error_list.h"
#include "cpe/utils/buffer.h"
#include "CfgTest.hpp"

class WriteTest : public CfgTest {
public:
    WriteTest();
    virtual void SetUp();
    virtual void TearDown();

    struct mem_buffer m_buffer;
    error_list_t m_errorList;

    int write(cfg_t cfg);
    const char * result(void);
};

#endif
