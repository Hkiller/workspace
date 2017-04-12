#include "gdpp/evt/tests-env/with_evt.hpp"

namespace Gd { namespace Evt { namespace testenv {

with_evt::with_evt() {
}

void with_evt::SetUp() {
    gd::evt::testenv::with_evt::SetUp();
}

void with_evt::TearDown() {
    gd::evt::testenv::with_evt::TearDown();
}

}}}

