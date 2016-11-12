#include "gd/app/tests-env/with_app.hpp"
#include "gd/timer/tests-env/with_timer.hpp"

namespace gd { namespace timer { namespace testenv {

with_timer::with_timer() {
}

void with_timer::SetUp() {
    gd_timer_mgr_t timer_mgr = 
        gd_timer_mgr_create(
            envOf<gd::app::testenv::with_app>().t_app(),
            NULL,
            NULL,
            t_allocrator(),
            NULL);
    EXPECT_TRUE(timer_mgr) << "crate default gd_timer_mgr fail!";
}

void with_timer::TearDown() {
}

gd_timer_mgr_t
with_timer::t_timer_mgr(const char * name) {
    gd_timer_mgr_t r = gd_timer_mgr_find_nc(
        envOf<gd::app::testenv::with_app>().t_app(),
        name);
    EXPECT_TRUE(r) << "gd_timer_mgr " << (name ? name : "default") 
                   << " not exist!";
    return r;
}

}}}
