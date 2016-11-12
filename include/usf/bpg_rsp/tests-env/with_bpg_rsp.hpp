#ifndef USF_BPG_RSP_TESTENV_WITH_BPG_H
#define USF_BPG_RSP_TESTENV_WITH_BPG_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../bpg_rsp_manage.h"
#include "../bpg_rsp.h"

namespace usf { namespace bpg { namespace testenv {

class with_bpg_rsp : public ::testenv::env<> {
public:
    with_bpg_rsp();

    void SetUp();
    void TearDown();

    bpg_rsp_manage_t t_bpg_rsp_manage(
        const char * name = NULL,
        const char * logic_name = NULL,
        const char * executor_mgr_name = NULL);

    logic_context_t t_bpg_context_create(dp_req_t pkg = NULL, const char * rsp_manage_name = NULL);
};

}}}

#endif
