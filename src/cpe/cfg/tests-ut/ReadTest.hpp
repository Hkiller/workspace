#ifndef CPE_DR_DATAJSON_TEST_PRINTTEST_H
#define CPE_DR_DATAJSON_TEST_PRINTTEST_H
#include <string.h>
#include "CfgTest.hpp"

class ReadTest
    : public testenv::fixture< ::Loki::NullType, CfgTest>
{
public:
    ReadTest();
    virtual void SetUp();
    virtual void TearDown();

    int read(const char * input, cfg_policy_t policy = cfg_replace);
    int read(cfg_t cfg, const char * input, cfg_policy_t policy = cfg_replace);

};

#endif
