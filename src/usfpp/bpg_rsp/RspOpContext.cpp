#include <cassert>
#include "gdpp/app/Log.hpp"
#include "usf/bpg_rsp/bpg_rsp_addition.h"
#include "usf/bpg_rsp/bpg_rsp_carry_info.h"
#include "usfpp/bpg_rsp/RspOpContext.hpp"

namespace Usf { namespace Bpg {

void RspOpContext::addAdditionData(uint32_t meta_id) {
    if (bpg_rsp_addition_data_add(*this, meta_id) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "add addition data %d fail!", meta_id);
    }
}

void RspOpContext::removeAdditionData(uint32_t meta_id) {
    bpg_rsp_addition_data_remove(*this, meta_id);
}

void RspOpContext::setCmd(uint32_t cmd) {
    bpg_rsp_carry_info_t carryInfo = bpg_rsp_carry_info_find(*this);
    if (carryInfo == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "RspOpContext[%d]: carry info not exist!", id());
    }

    bpg_rsp_context_set_cmd(carryInfo, cmd);
}

uint32_t RspOpContext::cmd(void) const {
    bpg_rsp_carry_info_t carryInfo = bpg_rsp_carry_info_find(*this);
    if (carryInfo == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "RspOpContext[%d]: carry info not exist!", id());
    }

    return bpg_rsp_context_cmd(carryInfo);
}

void RspOpContext::setSn(uint32_t sn) {
    bpg_rsp_carry_info_t carryInfo = bpg_rsp_carry_info_find(*this);
    if (carryInfo == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "RspOpContext[%d]: carry info not exist!", id());
    }

    bpg_rsp_context_set_sn(carryInfo, sn);
}

uint32_t RspOpContext::sn(void) const {
    bpg_rsp_carry_info_t carryInfo = bpg_rsp_carry_info_find(*this);
    if (carryInfo == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "RspOpContext[%d]: carry info not exist!", id());
    }

    return bpg_rsp_context_sn(carryInfo);
}

void RspOpContext::setClientId(uint64_t client_id) {
    bpg_rsp_carry_info_t carryInfo = bpg_rsp_carry_info_find(*this);
    if (carryInfo == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "RspOpContext[%d]: carry info not exist!", id());
    }

    bpg_rsp_context_set_client_id(carryInfo, client_id);
}

uint64_t RspOpContext::clientId(void) const {
    bpg_rsp_carry_info_t carryInfo = bpg_rsp_carry_info_find(*this);
    if (carryInfo == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "RspOpContext[%d]: carry info not exist!", id());
    }

    return bpg_rsp_context_client_id(carryInfo);
}

void RspOpContext::setResponse(bool needResponse) {
    bpg_rsp_carry_info_t carryInfo = bpg_rsp_carry_info_find(*this);
    if (carryInfo == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "RspOpContext[%d]: carry info not exist!", id());
    }

    bpg_rsp_context_set_no_response(carryInfo, needResponse ? 0 : 1);
}

}}
