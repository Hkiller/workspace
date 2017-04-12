#ifndef SVR_APPLE_IAP_SVR_OPS_H
#define SVR_APPLE_IAP_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "svr/center/agent/center_agent_types.h" 
#include "protocol/svr/apple_iap/svr_apple_iap_pro.h"
#include "apple_iap_svr_types.h"

/*operations of apple_iap_svr */
apple_iap_svr_t
apple_iap_svr_create(
    gd_app_context_t app,
    const char * name,
    net_trans_manage_t trans_mgr,
    set_svr_stub_t stub,
    mem_allocrator_t alloc,
    error_monitor_t em);

void apple_iap_svr_free(apple_iap_svr_t svr);

apple_iap_svr_t apple_iap_svr_find(gd_app_context_t app, cpe_hash_string_t name);
apple_iap_svr_t apple_iap_svr_find_nc(gd_app_context_t app, const char * name);
const char * apple_iap_svr_name(apple_iap_svr_t svr);

int apple_iap_svr_set_send_to(apple_iap_svr_t svr, const char * send_to);
int apple_iap_svr_set_request_recv_at(apple_iap_svr_t svr, const char * name);

dp_req_t apple_iap_svr_pkg_buf(apple_iap_svr_t svr, size_t capacity);

/*apple_iap request ops*/
void apple_iap_svr_request_validate(apple_iap_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);

#endif
