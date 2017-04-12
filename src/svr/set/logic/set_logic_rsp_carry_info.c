#include <assert.h>
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_context.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "protocol/set/logic/set_logic_rsp_carry_info.h"

set_logic_rsp_carry_info_t set_logic_rsp_carry_info_find(logic_context_t ctx) {
    logic_data_t data;
    data = logic_context_data_find(ctx, "set_logic_carry_info");

    return data == NULL
        ? NULL
        : (set_logic_rsp_carry_info_t)logic_data_data(data);
}

uint32_t set_logic_rsp_context_cmd(set_logic_rsp_carry_info_t carry_info) {
    assert(carry_info);
    return ((SET_LOGIC_CARRY_INFO *)carry_info)->cmd;
}

void set_logic_rsp_context_set_cmd(set_logic_rsp_carry_info_t carry_info, uint32_t cmd) {
    ((SET_LOGIC_CARRY_INFO *)carry_info)->cmd = cmd;
}

uint32_t set_logic_rsp_context_sn(set_logic_rsp_carry_info_t carry_info) {
    assert(carry_info);
    return ((SET_LOGIC_CARRY_INFO *)carry_info)->sn;
}

void set_logic_rsp_context_set_sn(set_logic_rsp_carry_info_t carry_info, uint32_t sn) {
    ((SET_LOGIC_CARRY_INFO *)carry_info)->sn = sn;
}

uint16_t set_logic_rsp_context_from_svr_id(set_logic_rsp_carry_info_t carry_info) {
    assert(carry_info);
    return ((SET_LOGIC_CARRY_INFO *)carry_info)->from_svr_id;
}

void set_logic_rsp_context_set_from_svr_id(set_logic_rsp_carry_info_t carry_info, uint16_t from_svr_id) {
    ((SET_LOGIC_CARRY_INFO *)carry_info)->from_svr_id = from_svr_id;
}

uint16_t set_logic_rsp_context_from_svr_type(set_logic_rsp_carry_info_t carry_info) {
    assert(carry_info);
    return ((SET_LOGIC_CARRY_INFO *)carry_info)->from_svr_type;
}

void set_logic_rsp_context_set_from_svr_type(set_logic_rsp_carry_info_t carry_info, uint16_t from_svr_type) {
    ((SET_LOGIC_CARRY_INFO *)carry_info)->from_svr_type = from_svr_type;
}

void set_logic_rsp_context_set_response(set_logic_rsp_carry_info_t carry_info, uint32_t response) {
    ((SET_LOGIC_CARRY_INFO *)carry_info)->response = response;
}

uint32_t set_logic_rsp_context_response(set_logic_rsp_carry_info_t carry_info) {
    return ((SET_LOGIC_CARRY_INFO *)carry_info)->response;
}

int set_logic_rsp_context_get_conn_info(logic_context_t ctx, uint16_t * from_svr_type, uint16_t * from_svr_id) {
    logic_data_t data;
    SET_LOGIC_CARRY_INFO * carry_info;

    data = logic_context_data_find(ctx, "set_logic_carry_info");
    if (data == NULL) return -1;
    carry_info = logic_data_data(data);

    if (from_svr_type) *from_svr_type = carry_info->from_svr_type;
    if (from_svr_id) *from_svr_id = carry_info->from_svr_id;

    return 0;
}
