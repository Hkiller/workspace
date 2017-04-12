#ifndef GD_DR_CVT_TESTENV_WITH_DR_CVT_H
#define GD_DR_CVT_TESTENV_WITH_DR_CVT_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../dr_cvt_manage.h"

namespace gd { namespace dr_cvt { namespace testenv {

class with_dr_cvt : public ::testenv::env<> {
public:
    void SetUp();
    void TearDown();

    void t_dr_cvt_install_pbuf(void);
    void t_dr_cvt_install_pbuf_len(void);
};

}}}

#endif
