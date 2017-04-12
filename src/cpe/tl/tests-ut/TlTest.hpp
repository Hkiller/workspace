#ifndef CPE_TL_TEST_TLTEST_H
#define CPE_TL_TEST_TLTEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/tl/tl.h"
#include "../tl_internal_ops.h"

class TlTest : public testenv::fixture<> {
public:
    TlTest();

    virtual void SetUp();
    virtual void TearDown();

    tl_manage_t m_manage;
    tl_t m_tl;

    void installTl(void);

    tl_event_t createEvent(size_t capacity);
    tl_event_t createAction();
};

#endif
