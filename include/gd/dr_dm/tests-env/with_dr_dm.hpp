#ifndef GD_DR_DM_TESTENV_WITH_MANAGE_H
#define GD_DR_DM_TESTENV_WITH_MANAGE_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../dr_dm_manage.h"

namespace gd { namespace dr_dm { namespace testenv {

class with_dr_dm : public ::testenv::env<> {
public:
    with_dr_dm();

    void SetUp();
    void TearDown();

    dr_dm_manage_t
    t_dr_dm_manage_create(
        const char * name,
        const char * metalib,
        const char * metaname,
        const char * id_attr_name = NULL,
        const char * id_generator = NULL);

    dr_dm_manage_t
    t_dr_dm_manage_create(
        const char * name,
        LPDRMETA meta = NULL,
        const char * id_attr_name = NULL,
        const char * id_generator = NULL);

    dr_dm_manage_t t_dr_dm_manage(const char * name);
};

}}}

#endif
