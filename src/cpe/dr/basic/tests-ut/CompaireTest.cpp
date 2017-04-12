#include "cpe/utils/string_utils.h"
#include "CompaireTest.hpp"

int CTypeCompaireTest::comp(int lType, const char * lValue, int rType, const char * rValue) {
    char lBuf[100];
    char rBuf[100];
    if (lType != CPE_DR_TYPE_STRING) {
        EXPECT_EQ(0, dr_ctype_set_from_string(lBuf, lType, lValue, 0));
    }
    else {
        cpe_str_dup(lBuf, sizeof(lBuf), lValue);
    }

    if (rType != CPE_DR_TYPE_STRING)  {
        EXPECT_EQ(0, dr_ctype_set_from_string(rBuf, rType, rValue, 0));
    }
    else {
        cpe_str_dup(rBuf, sizeof(rBuf), rValue);
    }

    return dr_ctype_cmp(lBuf, lType, rBuf, rType);
}

