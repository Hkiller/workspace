#ifndef GD_EVT_TESTENV_WITH_EVT_H
#define GD_EVT_TESTENV_WITH_EVT_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../evt_manage.h"

namespace gd { namespace evt { namespace testenv {

class with_evt : public ::testenv::env<> {
public:
    with_evt();

    void SetUp();
    void TearDown();

    void t_evt_mgr_set_metalib(const char * metalib);
    gd_evt_mgr_t t_evt_mgr(const char * name = 0);
};

}}}

#endif
