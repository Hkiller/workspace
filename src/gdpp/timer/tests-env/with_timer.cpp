#include "gdpp/timer/tests-env/with_timer.hpp"

namespace Gd { namespace Timer { namespace testenv {

with_timer::with_timer() {
}

void with_timer::SetUp() {
    gd::timer::testenv::with_timer::SetUp();
}

void with_timer::TearDown() {
    gd::timer::testenv::with_timer::TearDown();
}

}}}

