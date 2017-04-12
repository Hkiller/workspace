#ifndef CPE_DR_DATAPBUF_TEST_READTEST_H
#define CPE_DR_DATAPBUF_TEST_READTEST_H
#include <string.h>
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/dr/dr_bson.h"

typedef LOKI_TYPELIST_2(
    utils::testenv::with_em,
    cpe::cfg::testenv::with_cfg) ReadTestBase;

class ReadTest : public testenv::fixture<ReadTestBase> {
public:
    ReadTest();
    virtual void SetUp();
    virtual void TearDown();

    char m_result_buffer[1024];
    int m_result_bufffer_len;
    LPDRMETA m_result_meta;

    LPDRMETALIB m_metaLib;
    struct mem_buffer m_metaLib_buffer;

    int metaSize(const char * typeName);
    void installMeta(const char * data);

    int read(const char * typeName, const char * defs) { return read(typeName, typeName, defs); }
    int read(const char * encodeTypeName, const char * decodeTypeName, const char * defs);
    int read(const char * decodeTypeName, const void * data, size_t data_size);

    cfg_t result();
};

#endif
