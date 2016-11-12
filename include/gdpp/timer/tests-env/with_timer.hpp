#ifndef GDPP_TIMER_TESTENV_WITHTIMER_H
#define GDPP_TIMER_TESTENV_WITHTIMER_H
#include "gd/timer/tests-env/with_timer.hpp"
#include "../TimerCenter.hpp"

namespace Gd { namespace Timer { namespace testenv {

class with_timer : public gd::timer::testenv::with_timer {
public:
    with_timer();

    void SetUp();
    void TearDown();

    TimerCenter & t_timer_mgr_ex(const char * name = 0) { return TimerCenter::_cast(t_timer_mgr(name)); }
};

}}}

#endif
