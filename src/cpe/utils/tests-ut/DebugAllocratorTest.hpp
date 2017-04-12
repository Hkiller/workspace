#ifndef CPE_DR_TEST_DEBUGALLOCRATOR_H
#define CPE_DR_TEST_DEBUGALLOCRATOR_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/utils/memory_debug.h"

typedef LOKI_TYPELIST_1(
    utils::testenv::with_em) DebugAllocratorTestBase;

class DebugAllocratorTest : public testenv::fixture<DebugAllocratorTestBase> {
public:
    DebugAllocratorTest();

    virtual void SetUp(void);
    virtual void TearDown(void);

    mem_allocrator_t m_allocrator;

    const char * dump(void);
};

#endif

