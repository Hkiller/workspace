#ifndef GDPP_EVT_TESTENV_WITHEVT_H
#define GDPP_EVT_TESTENV_WITHEVT_H
#include "gd/evt/tests-env/with_evt.hpp"
#include "../Event.hpp"
#include "../EventCenter.hpp"

namespace Gd { namespace Evt { namespace testenv {

class with_evt : public gd::evt::testenv::with_evt {
public:
    with_evt();

    void SetUp();
    void TearDown();

    EventCenter & t_evt_mgr_ex(const char * name = 0) { return EventCenter::_cast(t_evt_mgr(name)); }
};

}}}

#endif
