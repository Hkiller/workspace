#ifndef CPE_DR_DATAJSON_TEST_PRINTTEST_H
#define CPE_DR_DATAJSON_TEST_PRINTTEST_H
#include <string.h>
#include "CfgTest.hpp"

class MergeTest
    : public testenv::fixture< ::Loki::NullType, CfgTest>
{
public:
    MergeTest();
    virtual void SetUp();
    virtual void TearDown();

    void install(const char * input);
    void install(cfg_t cfg, const char * input);

    int merge(const char * input, cfg_policy_t policy = cfg_replace);

    struct mem_buffer m_result_buffer;
    const char * result(void);
    const char * result(cfg_t cfg);
};

#endif
