#ifndef CPE_DR_DATAJSON_TEST_PRINTPATHTEST_H
#define CPE_DR_DATAJSON_TEST_PRINTPATHTEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/cfg/cfg.h"

class PrintPathTest
    : public testenv::fixture<LOKI_TYPELIST_1(cpe::cfg::testenv::with_cfg)>
{
public:
    PrintPathTest();

    virtual void TearDown();

    void installCfg(const char * def);

    const char * path(const char * p, const char * r = 0);

    cfg_t m_root;
};

#endif
