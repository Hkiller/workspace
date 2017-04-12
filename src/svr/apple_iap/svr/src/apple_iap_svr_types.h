#ifndef SVR_APPLE_IAP_SVR_TYPES_H
#define SVR_APPLE_IAP_SVR_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "gd/net_trans/net_trans_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "protocol/svr/apple_iap/svr_apple_iap_internal.h"

typedef struct apple_iap_svr * apple_iap_svr_t;

struct apple_iap_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    int m_debug;

    LPDRMETA m_meta_res_validate;
    LPDRMETA m_meta_res_error;

    net_trans_group_t m_trans_group;
    int8_t m_is_sandbox;

    dp_req_t m_outgoing_pkg;
    cpe_hash_string_t m_send_to;
    dp_rsp_t m_recv_at;
};

struct apple_iap_task_data {
    char m_receipt[SVR_APPLE_IAP_RECEIPT_MAX];
    uint32_t m_sn;
    uint16_t m_from_svr_type;
    uint16_t m_from_svr_id;
    uint16_t m_is_sandbox;
};

typedef void (*apple_iap_svr_op_t)(apple_iap_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);

#endif
