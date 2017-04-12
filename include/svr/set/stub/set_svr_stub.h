#ifndef SVR_SET_SVR_STUB_H
#define SVR_SET_SVR_STUB_H
#include "cpe/utils/hash_string.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "set_svr_stub_types.h"

#ifdef __cplusplus
extern "C" {
#endif

set_svr_stub_t set_svr_stub_find_nc(gd_app_context_t app, const char * name);
set_svr_stub_t set_svr_stub_find(gd_app_context_t app, cpe_hash_string_t name);

gd_app_context_t set_svr_stub_app(set_svr_stub_t mgr);
const char * set_svr_stub_name(set_svr_stub_t mgr);
cpe_hash_string_t set_svr_stub_name_hs(set_svr_stub_t mgr);

set_svr_svr_info_t set_svr_stub_svr_type(set_svr_stub_t mgr);
uint16_t set_svr_stub_svr_id(set_svr_stub_t mgr);

cpe_hash_string_t set_svr_stub_request_dispatch_to(set_svr_stub_t svr);
int set_svr_stub_set_request_dispatch_to(set_svr_stub_t svr, const char * dispatch_to);

cpe_hash_string_t set_svr_stub_svr_notify_dispatch_to(set_svr_stub_t svr, uint16_t svr_type);
int set_svr_stub_set_notify_dispatch_to(set_svr_stub_t svr, const char * dispatch_to);

cpe_hash_string_t set_svr_stub_response_dispatch_to(set_svr_stub_t svr);
int set_svr_stub_set_response_dispatch_to(set_svr_stub_t svr, const char * dispatch_to);

cpe_hash_string_t set_svr_stub_svr_response_dispatch_to(set_svr_stub_t svr, uint16_t svr_type);

int set_svr_stub_set_outgoing_recv_at(set_svr_stub_t svr, const char * outgoing_recv_at);

void set_svr_stub_set_recv_capacity(set_svr_stub_t svr, size_t capacity);

int set_svr_stub_start(set_svr_stub_t svr, uint16_t port);
void set_svr_stub_stop(set_svr_stub_t svr);

const char * set_svr_stub_name(set_svr_stub_t svr);

int set_svr_stub_read_data(
    set_svr_stub_t stub, set_svr_svr_info_t svr_info, dp_req_t pkg,
    uint32_t * cmd, LPDRMETA * meta, void ** data, size_t * data_size);

int set_svr_stub_send_pkg(set_svr_stub_t svr, dp_req_t body);

int set_svr_stub_send_req_pkg(
    set_svr_stub_t svr, uint16_t svr_type, uint16_t svr_id,
    uint16_t sn, dp_req_t body,
    void const * carry_data, size_t carry_data_len);

int set_svr_stub_send_req_data(
    set_svr_stub_t svr, uint16_t svr_type, uint16_t svr_id,
    uint32_t sn, void const * data, uint16_t data_size, LPDRMETA meta,
    void const * carry_data, size_t carry_data_len);

int set_svr_stub_send_req_cmd(
    set_svr_stub_t svr, uint16_t svr_type, uint16_t svr_id,
    uint32_t sn, uint32_t cmd,
    void const * carry_data, size_t carry_data_len);

int set_svr_stub_send_notify_pkg(
    set_svr_stub_t svr, uint16_t svr_type, uint16_t svr_id,
    uint16_t sn, dp_req_t body,
    void const * carry_data, size_t carry_data_len);

int set_svr_stub_send_notify_data(
    set_svr_stub_t svr, uint16_t svr_type, uint16_t svr_id,
    uint32_t sn, void const * data, uint16_t data_size, LPDRMETA meta,
    void const * carry_data, size_t carry_data_len);

int set_svr_stub_send_notify_cmd(
    set_svr_stub_t svr, uint16_t svr_type, uint16_t svr_id,
    uint32_t sn, uint32_t cmd,
    void const * carry_data, size_t carry_data_len);

int set_svr_stub_send_response_pkg(
    set_svr_stub_t svr, uint16_t svr_type, uint16_t svr_id,
    uint16_t sn, dp_req_t body,
    void const * carry_data, size_t carry_data_len);

int set_svr_stub_send_response_data(
    set_svr_stub_t svr, uint16_t svr_type, uint16_t svr_id,
    uint32_t sn, void const * data, uint16_t data_size, LPDRMETA meta,
    void const * carry_data, size_t carry_data_len);

int set_svr_stub_send_response_cmd(
    set_svr_stub_t svr, uint16_t svr_type, uint16_t svr_id,
    uint32_t sn, uint32_t cmd,
    void const * carry_data, size_t carry_data_len);


int set_svr_stub_reply_pkg(set_svr_stub_t svr, dp_req_t req, dp_req_t body);
int set_svr_stub_reply_data(set_svr_stub_t svr, dp_req_t req, void const * data, uint16_t data_size, LPDRMETA meta);
int set_svr_stub_reply_cmd(set_svr_stub_t svr, dp_req_t req, uint32_t cmd);

dp_req_t set_svr_stub_outgoing_pkg_buf(set_svr_stub_t stub, size_t capacity);
void * set_svr_stub_pkg_to_data(set_svr_stub_t stub, dp_req_t buf, uint16_t svr_type_id, LPDRMETA data_meta, size_t * data_capacity);

#ifdef __cplusplus
}
#endif

#endif
