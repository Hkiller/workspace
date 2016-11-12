#ifndef USF_BPG_RSP_CONTEXT_H
#define USF_BPG_RSP_CONTEXT_H
#include "bpg_rsp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

bpg_rsp_carry_info_t bpg_rsp_carry_info_find(logic_context_t ctx);

uint32_t bpg_rsp_context_cmd(bpg_rsp_carry_info_t carry_info);
void bpg_rsp_context_set_cmd(bpg_rsp_carry_info_t carry_info, uint32_t);

uint32_t bpg_rsp_context_sn(bpg_rsp_carry_info_t carry_info);
void bpg_rsp_context_set_sn(bpg_rsp_carry_info_t carry_info, uint32_t);

uint64_t bpg_rsp_context_client_id(bpg_rsp_carry_info_t carry_info);
void bpg_rsp_context_set_client_id(bpg_rsp_carry_info_t carry_info, uint64_t);

void bpg_rsp_context_set_no_response(bpg_rsp_carry_info_t carry_info, int no_response);

#ifdef __cplusplus
}
#endif

#endif
