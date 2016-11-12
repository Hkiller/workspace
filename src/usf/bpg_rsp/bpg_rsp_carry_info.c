#include <assert.h>
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_context.h"
#include "usf/bpg_rsp/bpg_rsp_carry_info.h"
#include "protocol/bpg_rsp/bpg_rsp_carry_info.h"

bpg_rsp_carry_info_t bpg_rsp_carry_info_find(logic_context_t ctx) {
    logic_data_t data;
    data = logic_context_data_find(ctx, "bpg_carry_info");

    return data == NULL
        ? NULL
        : (bpg_rsp_carry_info_t)logic_data_data(data);
}

uint32_t bpg_rsp_context_cmd(bpg_rsp_carry_info_t carry_info) {
    assert(carry_info);
    return ((BPG_CARRY_INFO *)carry_info)->cmd;
}

void bpg_rsp_context_set_cmd(bpg_rsp_carry_info_t carry_info, uint32_t cmd) {
    ((BPG_CARRY_INFO *)carry_info)->cmd = cmd;
}

uint32_t bpg_rsp_context_sn(bpg_rsp_carry_info_t carry_info) {
    assert(carry_info);
    return ((BPG_CARRY_INFO *)carry_info)->sn;
}

void bpg_rsp_context_set_sn(bpg_rsp_carry_info_t carry_info, uint32_t sn) {
    ((BPG_CARRY_INFO *)carry_info)->sn = sn;
}

uint64_t bpg_rsp_context_client_id(bpg_rsp_carry_info_t carry_info) {
    return ((BPG_CARRY_INFO *)carry_info)->clientId;
}

void bpg_rsp_context_set_client_id(bpg_rsp_carry_info_t carry_info, uint64_t client_id) {
    ((BPG_CARRY_INFO *)carry_info)->clientId = client_id;
}

void bpg_rsp_context_set_no_response(bpg_rsp_carry_info_t carry_info, int no_response) {
    ((BPG_CARRY_INFO *)carry_info)->no_response = no_response;
}
