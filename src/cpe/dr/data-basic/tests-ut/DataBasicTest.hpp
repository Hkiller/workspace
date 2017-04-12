#ifndef CPE_DR_DATABASIC_TEST_H
#define CPE_DR_DATABASIC_TEST_H
#include <string.h>
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/dr/dr_data.h"

typedef LOKI_TYPELIST_1(
    utils::testenv::with_em) DataBasicTestBase;

class DataBasicTest : public testenv::fixture<DataBasicTestBase> {
public:
    DataBasicTest();
    virtual void SetUp();
    virtual void TearDown();

    LPDRMETALIB m_metaLib;
    struct mem_buffer m_metaLib_buffer;

    struct mem_buffer m_buffer;

    void installMeta(const char * data);
    void * result(int startPos = 0);
};

#endif
