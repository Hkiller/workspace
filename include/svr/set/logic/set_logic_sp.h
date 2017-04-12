#ifndef SVR_SET_LOGIC_SP_H
#define SVR_SET_LOGIC_SP_H
#include "cpe/utils/hash_string.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "set_logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

set_logic_sp_t set_logic_sp_create(
    gd_app_context_t app,
    const char * name, 
    logic_manage_t mgr,
    set_svr_stub_t stub,
    mem_allocrator_t alloc, error_monitor_t em);

void set_logic_sp_free(set_logic_sp_t mgr);

set_logic_sp_t set_logic_sp_find(gd_app_context_t app, cpe_hash_string_t name);
set_logic_sp_t set_logic_sp_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t set_logic_sp_app(set_logic_sp_t mgr);
const char * set_logic_sp_name(set_logic_sp_t mgr);
cpe_hash_string_t set_logic_sp_name_hs(set_logic_sp_t mgr);

set_svr_stub_t set_logic_sp_stub(set_logic_sp_t mgr);

int set_logic_sp_set_outgoing_dispatch_to(set_logic_sp_t sp, const char * outgoing_dispatch_to);
cpe_hash_string_t set_logic_sp_outgoing_dispatch_to(set_logic_sp_t sp);
int set_logic_sp_set_incoming_recv_at(set_logic_sp_t sp, const char * incoming_recv_at);

int set_logic_sp_send_pkg(set_logic_sp_t sp, dp_req_t pkg, logic_require_t require);

int set_logic_sp_send_req_pkg(
    set_logic_sp_t sp, uint16_t to_svr_type, uint16_t to_svr_id,
    dp_req_t pkg,
    void const * carry_data, size_t carry_data_size,
    logic_require_t require);

int set_logic_sp_send_req_data(
    set_logic_sp_t sp, uint16_t to_svr_type, uint16_t to_svr_id,
    LPDRMETA meta, void const * data, size_t data_size,
    void const * carry_data, size_t carry_data_size,
    logic_require_t require);

int set_logic_sp_send_req_cmd(
    set_logic_sp_t sp, uint16_t to_svr_type, uint16_t to_svr_id,
    uint32_t cmd,
    void const * carry_data, size_t carry_data_size,
    logic_require_t require);

int set_logic_sp_response_from_svr_type(logic_require_t require, uint16_t * svr_type);
int set_logic_sp_response_from_svr_id(logic_require_t require, uint16_t * svr_id);

#ifdef __cplusplus
}
#endif

#endif
