#include <cassert>
#include "gdpp/app/Log.hpp"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "svrpp/set/logic/RspOpContext.hpp"

namespace Svr { namespace Set {

void RspOpContext::setCmd(uint32_t cmd) {
    set_logic_rsp_carry_info_t carryInfo = set_logic_rsp_carry_info_find(*this);
    if (carryInfo == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "RspOpContext[%d]: carry info not exist!", id());
    }

    set_logic_rsp_context_set_cmd(carryInfo, cmd);
}

uint32_t RspOpContext::cmd(void) const {
    set_logic_rsp_carry_info_t carryInfo = set_logic_rsp_carry_info_find(*this);
    if (carryInfo == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "RspOpContext[%d]: carry info not exist!", id());
    }

    return set_logic_rsp_context_cmd(carryInfo);
}

void RspOpContext::setSn(uint32_t sn) {
    set_logic_rsp_carry_info_t carryInfo = set_logic_rsp_carry_info_find(*this);
    if (carryInfo == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "RspOpContext[%d]: carry info not exist!", id());
    }

    set_logic_rsp_context_set_sn(carryInfo, sn);
}

uint32_t RspOpContext::sn(void) const {
    set_logic_rsp_carry_info_t carryInfo = set_logic_rsp_carry_info_find(*this);
    if (carryInfo == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "RspOpContext[%d]: carry info not exist!", id());
    }

    return set_logic_rsp_context_sn(carryInfo);
}

void RspOpContext::setResponse(uint32_t response) {
    set_logic_rsp_carry_info_t carryInfo = set_logic_rsp_carry_info_find(*this);
    if (carryInfo == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "RspOpContext[%d]: carry info not exist!", id());
    }

    set_logic_rsp_context_set_response(carryInfo, response);
}

uint16_t RspOpContext::fromSvrType(void) const {
    set_logic_rsp_carry_info_t carryInfo = set_logic_rsp_carry_info_find(*this);
    if (carryInfo == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "RspOpContext[%d]: carry info not exist!", id());
    }

    return set_logic_rsp_context_from_svr_type(carryInfo);
}

uint16_t RspOpContext::fromSvrId(void) const {
    set_logic_rsp_carry_info_t carryInfo = set_logic_rsp_carry_info_find(*this);
    if (carryInfo == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "RspOpContext[%d]: carry info not exist!", id());
    }

    return set_logic_rsp_context_from_svr_id(carryInfo);
}

void RspOpContext::setFromSvr(uint16_t from_svr_type, uint16_t from_svr_id) {
    set_logic_rsp_carry_info_t carryInfo = set_logic_rsp_carry_info_find(*this);
    if (carryInfo == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "RspOpContext[%d]: carry info not exist!", id());
    }

    set_logic_rsp_context_set_from_svr_type(carryInfo, from_svr_type);
    set_logic_rsp_context_set_from_svr_id(carryInfo, from_svr_id);
}

}}
