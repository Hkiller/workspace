#include "TimerCenterTest.hpp"

void TimerCenterTest::SetUp() {
    Base::SetUp();
    t_app_set_timer_source_last_event();
}

void TimerCenterTest::TearDown() {
    Base::TearDown();
}
