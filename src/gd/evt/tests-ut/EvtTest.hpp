#ifndef GD_EVT_TEST_EVTTEST_H
#define GD_EVT_TEST_EVTTEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/utils/buffer.h"
#include "gd/app/tests-env/with_app.hpp"
#include "gd/dr_store/tests-env/with_dr_store.hpp"
#include "gd/evt/tests-env/with_evt.hpp"
#include "gd/evt/evt_manage.h"
#include "gd/evt/evt_read.h"

typedef LOKI_TYPELIST_4(
    utils::testenv::with_em,
    gd::app::testenv::with_app,
    gd::dr_store::testenv::with_dr_store,
    gd::evt::testenv::with_evt) EvtTestBase;

class EvtTest : public testenv::fixture<EvtTestBase> {
public:
    gd_evt_t createEvt(const char * typeName, size_t carry_size = 0, ssize_t data_capacity = -1);
};

#endif
