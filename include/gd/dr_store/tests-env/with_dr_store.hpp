#ifndef GD_DR_STORE_TESTENV_WITH_DR_STORE_H
#define GD_DR_STORE_TESTENV_WITH_DR_STORE_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../dr_store_manage.h"

namespace gd { namespace dr_store { namespace testenv {

class with_dr_store : public ::testenv::env<> {
public:
    with_dr_store();

    void SetUp();
    void TearDown();

    void t_dr_store_install(const char * libname, const char * def, uint8_t dft_align = 0);
    void t_dr_store_install(const char * libname, LPDRMETALIB metalib);
    void t_dr_store_reset(const char * libname, const char * def, uint8_t dft_align = 0);
    void t_dr_store_reset(const char * libname, LPDRMETALIB metalib);

    LPDRMETALIB t_metalib(const char * libname);
    LPDRMETA t_meta(const char * libname, const char * metaname);

private:
    dr_store_manage_t m_dr_store_mgr;
};

}}}

#endif
