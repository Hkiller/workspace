#ifndef CPE_DR_DATACFG_TEST_WRITETEST_H
#define CPE_DR_DATACFG_TEST_WRITETEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/dr/dr_cfg.h"

typedef LOKI_TYPELIST_2(
    utils::testenv::with_em,
    cpe::cfg::testenv::with_cfg) WriteTestBase;

class WriteTest : public testenv::fixture<WriteTestBase> {
public:
    WriteTest();
    virtual void SetUp();
    virtual void TearDown();

    LPDRMETALIB m_metaLib;
    struct mem_buffer m_metaLib_buffer;

    cfg_t m_cfg;

    void installMeta(const char * data);
    int write(const char * data, const char * typeName);
};

#endif
