#include <sstream>
#include <stdexcept>
#include "cpe/pal/pal_stackbuf.h"
#include "cpepp/utils/ErrorCollector.hpp"
#include "cpe/dp/dp_request.h"
#include "cpepp/dp/Manager.hpp"
#include "cpepp/dp/Responser.hpp"

namespace Cpe { namespace Dp {

void Manager::bind(dp_rsp_t rsp, int32_t cmd) {
    Cpe::Utils::ErrorCollector ec;
    if (dp_rsp_bind_numeric(rsp, cmd, ec) != 0) {
        ec.checkThrowWithMsg< ::std::runtime_error>();
    }
}

void Manager::bind(dp_rsp_t rsp, const char * cmd) {
    Cpe::Utils::ErrorCollector ec;
    if (dp_rsp_bind_string(rsp, cmd, ec) != 0) {
        ec.checkThrowWithMsg< ::std::runtime_error>();
    }
}

void Manager::unbind(dp_rsp_t rsp, int32_t cmd) {
    dp_rsp_unbind_numeric(rsp, cmd);
}

void Manager::unbind(dp_rsp_t rsp, const char * cmd) {
    dp_rsp_unbind_string(rsp, cmd);
}

void Manager::dispatch(const char * cmd, dp_req_t req, dp_mgr_t dm) {
    size_t cmdLen = strlen(cmd) + 1;
    char buf[CPE_STACK_BUF_LEN(cpe_hs_len_to_binary_len(cmdLen))];
    cpe_hs_init((cpe_hash_string_t)buf, sizeof(buf), cmd);

    dispatch((cpe_hash_string_t)buf, req, dm);
}

void Manager::dispatch(cpe_hash_string_t cmd, dp_req_t req, dp_mgr_t dm) {
    Cpe::Utils::ErrorCollector ec;

    if (dm == NULL) dm = dp_req_mgr(req);

    if (dp_dispatch_by_string(cmd, dm, req, ec) != 0) {
        ec.checkThrowWithMsg< ::std::runtime_error>();
    }
}

void Manager::dispatch(int32_t cmd, dp_req_t req, dp_mgr_t dm) {
    Cpe::Utils::ErrorCollector ec;

    if (dm == NULL) dm = dp_req_mgr(req);

    if (dp_dispatch_by_numeric(cmd, dm, req, ec) != 0) {
        ec.checkThrowWithMsg< ::std::runtime_error>();
    }
}

Responser & Manager::createResponser(const char * name) {
    dp_rsp_t rsp = dp_rsp_create(*this, name);
    if (rsp == NULL) {
        ::std::ostringstream os;
        os << "create rsp " << name << " fail!";
        throw ::std::runtime_error(os.str());
    }

    return *(Responser*)rsp;
}

void Manager::deleteResponser(const char * name) {
    dp_rsp_t rsp = dp_rsp_find_by_name(*this, name);
    if (rsp) dp_rsp_free(rsp);
}

Responser & Manager::rspByName(const char * name) {
    Responser * r = findRspByName(name);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "rsp " << name << " not exist!";
        throw ::std::runtime_error(os.str());
    }

    return *r;
}

Responser const & Manager::rspByName(const char * name) const {
    Responser const * r = findRspByName(name);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "rsp " << name << " not exist!";
        throw ::std::runtime_error(os.str());
    }

    return *r;
}

Responser & Manager::firstRsp(const char * cmd) {
    Responser * r = findFirstRsp(cmd);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "rsp to cmd " << cmd << " not exist!";
        throw ::std::runtime_error(os.str());
    }
    return *r;
}

Responser const & Manager::firstRsp(const char * cmd) const {
    Responser const * r = findFirstRsp(cmd);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "rsp to cmd " << cmd << " not exist!";
        throw ::std::runtime_error(os.str());
    }
    return *r;
}

Responser &
Manager::firstRsp(int32_t cmd) {
    Responser * r = findFirstRsp(cmd);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "rsp to cmd " << cmd << " not exist!";
        throw ::std::runtime_error(os.str());
    }
    return *r;
}

Responser const &
Manager::firstRsp(int32_t cmd) const {
    Responser const * r = findFirstRsp(cmd);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "rsp to cmd " << cmd << " not exist!";
        throw ::std::runtime_error(os.str());
    }
    return *r;
}

}}
