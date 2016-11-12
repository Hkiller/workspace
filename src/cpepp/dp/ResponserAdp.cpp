#include <stdexcept>
#include "cpe/utils/error.h"
#include "cpepp/utils/ErrorCollector.hpp"
#include "cpe/dp/dp_manage.h"
#include "cpepp/dp/Request.hpp"
#include "cpepp/dp/ResponserAdp.hpp"

namespace Cpe { namespace Dp {

int ResponserAdp::dp_rsp_adp_process(dp_req_t req, void * ctx, error_monitor_t em) {
    ResponserAdp * adp = (ResponserAdp *)ctx;
    try {
        adp->processRequest(Request::_cast(req), em);
    }
    catch(::std::exception const & e) {
        CPE_ERROR(em, "%s: ResponserAdp: process catch exception: %s", dp_rsp_name(adp->m_dp_rsp), e.what());
    }
    catch(...) {
        CPE_ERROR(em, "%s: ResponserAdp: process catch unknown exception", dp_rsp_name(adp->m_dp_rsp));
    }

    return 0;
}

ResponserAdp::ResponserAdp(dp_mgr_t mgr, const char * name) 
    : m_dp_rsp(dp_rsp_create(mgr, name))
{
    if (m_dp_rsp == NULL) {
        throw ::std::runtime_error("create ResponserAdp fail!");
    }

    dp_rsp_set_processor(m_dp_rsp, dp_rsp_adp_process, this);
}

ResponserAdp::~ResponserAdp() {
    dp_rsp_free(m_dp_rsp);
}

void ResponserAdp::bind(const char * targt) {
    Cpe::Utils::ErrorCollector ec;
    if (dp_rsp_bind_string(m_dp_rsp, targt, ec) != 0) {
        ec.checkThrowWithMsg< ::std::runtime_error>();
    }
}

void ResponserAdp::bind(int32_t target) {
    Cpe::Utils::ErrorCollector ec;
    if (dp_rsp_bind_numeric(m_dp_rsp, target, ec) != 0) {
        ec.checkThrowWithMsg< ::std::runtime_error>();
    }
}

void ResponserAdp::bind(cfg_t cfg) {
    Cpe::Utils::ErrorCollector ec;
    if (dp_rsp_bind_by_cfg(m_dp_rsp, cfg, ec) != 0) {
        ec.checkThrowWithMsg< ::std::runtime_error>();
    }
}

}}


