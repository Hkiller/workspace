#ifndef CPE_DR_DATACFG_TEST_READTEST_H
#define CPE_DR_DATACFG_TEST_READTEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/dr/dr_cfg.h"

typedef LOKI_TYPELIST_2(
    utils::testenv::with_em,
    cpe::cfg::testenv::with_cfg) ReadTestBase;

class ReadTest : public testenv::fixture<ReadTestBase> {
public:
    ReadTest();
    virtual void SetUp();
    virtual void TearDown();

    LPDRMETALIB m_metaLib;
    struct mem_buffer m_metaLib_buffer;

    struct mem_buffer m_buffer;

    void installMeta(const char * data);
    int read(const char * data, const char * typeName, int policy = 0, size_t capacity = 0);
    void * result(int startPos = 0);
};

#endif
