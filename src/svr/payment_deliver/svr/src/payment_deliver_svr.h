#ifndef SVR_PAYMENT_DELIVER_SVR_TYPES_H
#define SVR_PAYMENT_DELIVER_SVR_TYPES_H
#include "ebb.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/ringbuffer.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "protocol/svr/payment/svr_payment_pro.h"

typedef struct payment_deliver_svr * payment_deliver_svr_t;
typedef struct payment_deliver_connection * payment_deliver_connection_t;
typedef struct payment_deliver_request * payment_deliver_request_t;
typedef struct payment_deliver_adapter_type * payment_deliver_adapter_type_t;
typedef struct payment_deliver_adapter * payment_deliver_adapter_t;

typedef TAILQ_HEAD(payment_deliver_connection_list, payment_deliver_connection) payment_deliver_connection_list_t;
typedef TAILQ_HEAD(payment_deliver_request_list, payment_deliver_request) payment_deliver_request_list_t;
typedef TAILQ_HEAD(payment_deliver_adapter_list, payment_deliver_adapter) payment_deliver_adapter_list_t;

struct payment_deliver_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    int m_debug;

    ebb_server m_ebb_svr;
    uint16_t m_port;
    set_svr_svr_info_t m_payment_svr;

    LPDRMETA m_meta_req_notify;
    LPDRMETA m_meta_qihoo_record;
    LPDRMETA m_meta_iapppay_record;
    LPDRMETA m_meta_damai_record;

    uint32_t m_max_conn_id;
    uint32_t m_max_request_id;

    dp_rsp_t m_response_recv_at;
    payment_deliver_adapter_list_t m_adapters;

    ringbuffer_t m_ringbuf;

    payment_deliver_connection_list_t m_connections;
    struct cpe_hash_table m_requests;
};

typedef void (*payment_deliver_svr_op_t)(payment_deliver_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);

/*operations of payment_deliver_svr */
payment_deliver_svr_t
payment_deliver_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    set_svr_svr_info_t payment_svr,
    uint16_t port,
    mem_allocrator_t alloc,
    error_monitor_t em);

void payment_deliver_svr_free(payment_deliver_svr_t svr);

payment_deliver_svr_t payment_deliver_svr_find(gd_app_context_t app, cpe_hash_string_t name);
payment_deliver_svr_t payment_deliver_svr_find_nc(gd_app_context_t app, const char * name);
const char * payment_deliver_svr_name(payment_deliver_svr_t svr);
int payment_deliver_svr_set_ringbuf_size(payment_deliver_svr_t svr, size_t capacity);

int payment_deliver_svr_set_request_recv_at(payment_deliver_svr_t svr, const char * name);
int payment_deliver_svr_set_response_recv_at(payment_deliver_svr_t svr, const char * name);

SVR_PAYMENT_REQ_NOTIFY *
payment_deliver_svr_notify_pkg(
    payment_deliver_svr_t svr, dp_req_t * pkg, uint16_t service, uint8_t device_category, const char * trade_id);

#endif
