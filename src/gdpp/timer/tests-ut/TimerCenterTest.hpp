#ifndef GDPP_TIMER_TEST_TIMERCENTERTEST_H
#define GDPP_TIMER_TEST_TIMERCENTERTEST_H
#include "gmock/gmock.h"
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "gdpp/app/tests-env/with_app.hpp"
#include "gdpp/timer/tests-env/with_timer.hpp"
#include "gdpp/timer/TimerProcessor.hpp"
#include "gdpp/timer/TimerCenter.hpp"

typedef LOKI_TYPELIST_3(
    utils::testenv::with_em,
    Gd::App::testenv::with_app,
    Gd::Timer::testenv::with_timer) TimerCenterTestBase;

class TimerCenterTest : public testenv::fixture<TimerCenterTestBase> {
public:
    class TimerProcessorMock : public Gd::Timer::TimerProcessor {
    public:
        MOCK_METHOD1(on_timer1, void(Gd::Timer::TimerID));
        MOCK_METHOD1(on_timer2, void(Gd::Timer::TimerID));
    };

    virtual void SetUp();
    virtual void TearDown();

    void tick(void);
};

#endif
