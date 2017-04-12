#ifndef GD_TIMER_TESTENV_WITH_TIMER_H
#define GD_TIMER_TESTENV_WITH_TIMER_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../timer_manage.h"

namespace gd { namespace timer { namespace testenv {

class with_timer : public ::testenv::env<> {
public:
    with_timer();

    void SetUp();
    void TearDown();

    gd_timer_mgr_t t_timer_mgr(const char * name = 0);
};

}}}

#endif
