#ifndef SVR_SET_LOGIC_BPG_RSP_CONTEXT_H
#define SVR_SET_LOGIC_BPG_RSP_CONTEXT_H
#include "set_logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

set_logic_rsp_carry_info_t set_logic_rsp_carry_info_find(logic_context_t ctx);

uint32_t set_logic_rsp_context_cmd(set_logic_rsp_carry_info_t carry_info);
void set_logic_rsp_context_set_cmd(set_logic_rsp_carry_info_t carry_info, uint32_t cmd);

uint32_t set_logic_rsp_context_response(set_logic_rsp_carry_info_t carry_info);
void set_logic_rsp_context_set_response(set_logic_rsp_carry_info_t carry_info, uint32_t response);

uint32_t set_logic_rsp_context_sn(set_logic_rsp_carry_info_t carry_info);
void set_logic_rsp_context_set_sn(set_logic_rsp_carry_info_t carry_info, uint32_t);

uint16_t set_logic_rsp_context_from_svr_id(set_logic_rsp_carry_info_t carry_info);
void set_logic_rsp_context_set_from_svr_id(set_logic_rsp_carry_info_t carry_info, uint16_t from_svr_id);

uint16_t set_logic_rsp_context_from_svr_type(set_logic_rsp_carry_info_t carry_info);
void set_logic_rsp_context_set_from_svr_type(set_logic_rsp_carry_info_t carry_info, uint16_t from_svr_type);

int set_logic_rsp_context_get_conn_info(logic_context_t ctx, uint16_t * from_svr_type, uint16_t * from_svr_id);

#ifdef __cplusplus
}
#endif

#endif
