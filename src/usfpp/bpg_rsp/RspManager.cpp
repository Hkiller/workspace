#include <cassert>
#include "cpepp/utils/ErrorCollector.hpp"
#include "gdpp/app/Log.hpp"
#include "usf/bpg_rsp/bpg_rsp.h"
#include "usfpp/bpg_rsp/RspManager.hpp"

namespace Usf { namespace Bpg {

RspManager & RspManager::_cast(bpg_rsp_manage_t rsp_manage) {
    if (rsp_manage == NULL) {
        throw ::std::runtime_error("Usf::Bpg::RspManager::_cast: input rsp_manage is NULL!");
    }

    return *(RspManager*)rsp_manage;
}

RspManager & RspManager::instance(gd_app_context_t app, const char * name) {
    bpg_rsp_manage_t rsp_manage = bpg_rsp_manage_find_nc(app, name);
    if (rsp_manage == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "bpg_rsp_manage %s not exist!", name ? name : "default");
    }

    return *(RspManager*)rsp_manage;
}

bool RspManager::hasOp(const char * rspName) const {
    return bpg_rsp_find(*this, rspName) != NULL;
}

RspOpContext &
RspManager::createOp(const char * rspName, logic_context_t from) {
    Cpe::Utils::ErrorCollector ec;

    logic_context_t context = 
        bpg_rsp_manage_create_op_by_name(
            *this,
            from,
            rspName,
            ec);

    if (context == NULL) {
        ec.checkThrowWithMsg< ::std::runtime_error>("Usf::Bpg::RspManager::createOp: ");
    }

    assert(context);
    return *(RspOpContext*)context;
}

RspOpContext *
RspManager::tryCreateOp(const char * rspName, logic_context_t from) {
    logic_context_t context = 
        bpg_rsp_manage_create_op_by_name(
            *this,
            from,
            rspName,
            NULL);
    return (RspOpContext*)context;
}

RspOpContext &
RspManager::createFollowOp(logic_context_t context, const char * rspName) {
    Cpe::Utils::ErrorCollector ec;

    logic_context_t follow_context = 
        bpg_rsp_manage_create_follow_op_by_name(
            *this,
            context,
            rspName,
            ec);

    if (follow_context == NULL) {
        ec.checkThrowWithMsg< ::std::runtime_error>("Usf::Bpg::RspManager::createFollowOp: ");
    }

    assert(follow_context);
    return *(RspOpContext*)follow_context;
}

RspOpContext *
RspManager::tryCreateFollowOp(logic_context_t context, const char * rspName) {
    logic_context_t follow_context = 
        bpg_rsp_manage_create_follow_op_by_name(
            *this,
            context,
            rspName,
            NULL);
    return (RspOpContext*)follow_context;
}

void RspManager::loadRsps(cfg_t cfg, LPDRMETALIB metalib) {
    Cpe::Utils::ErrorCollector ec;

    if (bpg_rsp_build(*this, cfg, metalib, ec) != 0) {
        ec.checkThrowWithMsg< ::std::runtime_error>("Usf::Bpg::RspManager::loadRsps: ");
    }
}

}}
