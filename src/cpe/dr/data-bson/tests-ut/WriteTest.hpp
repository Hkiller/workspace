#ifndef CPE_DR_DATAPBUF_TEST_WRITETEST_H
#define CPE_DR_DATAPBUF_TEST_WRITETEST_H
#include <string.h>
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/dr/dr_bson.h"

typedef LOKI_TYPELIST_2(
    utils::testenv::with_em,
    cpe::cfg::testenv::with_cfg) WriteTestBase;

class WriteTest : public testenv::fixture<WriteTestBase> {
public:
    WriteTest();
    virtual void SetUp();
    virtual void TearDown();

    char m_buffer[1024];
    int m_bufffer_len;

    LPDRMETALIB m_metaLib;
    struct mem_buffer m_metaLib_buffer;

    void installMeta(const char * data);

    int write(const char * typeName, const char * defs, uint8_t is_open = 0);
    int write(const char * typeName, const void * data, size_t data_size, uint8_t is_open = 0);
    const char * result(void);
};

#endif
