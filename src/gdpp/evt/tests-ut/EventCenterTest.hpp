#ifndef GDPP_EVT_TEST_EVENTCENTERTEST_H
#define GDPP_EVT_TEST_EVENTCENTERTEST_H
#include "gmock/gmock.h"
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "gdpp/app/tests-env/with_app.hpp"
#include "gd/dr_store/tests-env/with_dr_store.hpp"
#include "gdpp/evt/tests-env/with_evt.hpp"
#include "gdpp/evt/EventResponser.hpp"
#include "gdpp/evt/Event.hpp"
#include "gdpp/evt/EventCenter.hpp"

typedef LOKI_TYPELIST_4(
    utils::testenv::with_em,
    Gd::App::testenv::with_app,
    gd::dr_store::testenv::with_dr_store,
    Gd::Evt::testenv::with_evt) EventCenterTestBase;

class EventCenterTest : public testenv::fixture<EventCenterTestBase> {
public:
    class EventResponserMock : public Gd::Evt::EventResponser {
    public:
        MOCK_METHOD2(on_event1, void(const char * oid, Gd::Evt::Event const & e));
        MOCK_METHOD2(on_event2, void(const char * oid, Gd::Evt::Event const & e));
    };

    virtual void SetUp();
    virtual void TearDown();

    void tick(void);
};

#endif
