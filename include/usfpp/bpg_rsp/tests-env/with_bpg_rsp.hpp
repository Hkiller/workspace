#ifndef USFPP_BPG_RSP_TESTENV_WITH_BPG_H
#define USFPP_BPG_RSP_TESTENV_WITH_BPG_H
#include "usf/bpg_rsp/tests-env/with_bpg_rsp.hpp"
#include "../RspManager.hpp"
#include "../RspOpContext.hpp"

namespace Usf { namespace Bpg { namespace testenv {

class with_bpg_rsp : public usf::bpg::testenv::with_bpg_rsp {
public:
    with_bpg_rsp();

    void SetUp();
    void TearDown();

    RspManager & t_bpg_rsp_manage_ex(const char * name = NULL, const char * logic_name = NULL) {
        return RspManager::_cast(t_bpg_rsp_manage(name, logic_name));
    }

    RspOpContext & t_bpg_context_create_ex(dp_req_t pkg = NULL, const char * rsp_manage_name = NULL) {
        return *(RspOpContext*)t_bpg_context_create(pkg, rsp_manage_name);
    }
};

}}}

#endif
