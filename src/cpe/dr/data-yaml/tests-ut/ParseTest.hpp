#ifndef CPE_DR_DATAYAML_TEST_PARSETEST_H
#define CPE_DR_DATAYAML_TEST_PARSETEST_H
#include <string.h>
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/dr/dr_yaml.h"

typedef LOKI_TYPELIST_1(
    utils::testenv::with_em) ParseTestBase;

class ParseTest : public testenv::fixture<ParseTestBase> {
public:
    ParseTest();
    virtual void SetUp();
    virtual void TearDown();

    LPDRMETALIB m_metaLib;
    struct mem_buffer m_metaLib_buffer;

    struct mem_buffer m_buffer;

    int metaSize(const char * typeName);
    void installMeta(const char * data);
    int read(const char * data, const char * typeName);
    void * result(int startPos = 0);
};

#define ASSERT_YAML_READ_RESULT(__expect)  do {                 \
    ASSERT_TRUE(result()) << "no data output!";                 \
    ASSERT_EQ(sizeof(__expect), mem_buffer_size(&m_buffer))     \
        << "output size error!" ;                               \
    ASSERT_EQ(0, memcmp(&__expect, result(), sizeof(__expect))) \
        << "data error!";                                       \
    } while(0)

#endif
