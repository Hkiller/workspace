#ifndef CPE_DR_DATAJSON_TEST_PRINTTEST_H
#define CPE_DR_DATAJSON_TEST_PRINTTEST_H
#include <string.h>
#include "CfgTest.hpp"

class BinTest
    : public testenv::fixture< ::Loki::NullType, CfgTest>
{
public:
    BinTest();
    virtual void SetUp();
    virtual void TearDown();

    cfg_t build_by_bin(cfg_t cfg);
    cfg_t build_by_bin_file(const char * path);
    cfg_t build_by_bin(int typeId, const char * value);
    cfg_t build_by_bin(const char * value);
};

#endif
