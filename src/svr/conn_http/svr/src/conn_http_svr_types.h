#ifndef SVR_CONN_HTTP_SVR_TYPES_H
#define SVR_CONN_HTTP_SVR_TYPES_H
#include "ebb.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/ringbuffer.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "protocol/svr/conn_http/svr_conn_http_internal.h"

typedef struct conn_http_svr * conn_http_svr_t;
typedef struct conn_http_service * conn_http_service_t;
typedef struct conn_http_cmd * conn_http_cmd_t;
typedef struct conn_http_connection * conn_http_connection_t;
typedef struct conn_http_request * conn_http_request_t;
typedef struct conn_http_formator * conn_http_formator_t;

typedef TAILQ_HEAD(conn_http_cmd_list, conn_http_cmd) conn_http_cmd_list_t;
typedef TAILQ_HEAD(conn_http_service_list, conn_http_service) conn_http_service_list_t;
typedef TAILQ_HEAD(conn_http_connection_list, conn_http_connection) conn_http_connection_list_t;
typedef TAILQ_HEAD(conn_http_request_list, conn_http_request) conn_http_request_list_t;

struct conn_http_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    int m_debug;

    ebb_server m_ebb_svr;
    uint16_t m_port;

    uint32_t m_max_conn_id;
    uint32_t m_max_request_id;

    LPDRMETA m_meta_res_error;

    dp_rsp_t m_request_recv_at;
    dp_rsp_t m_response_recv_at;

    ringbuffer_t m_ringbuf;

    conn_http_service_list_t m_services;
    conn_http_connection_list_t m_connections;
    struct cpe_hash_table m_requests;
};

struct conn_http_formator {
    void (*on_request)(conn_http_request_t request, const void * data, size_t data_len);
    void (*on_response)(conn_http_request_t request, const void * data, size_t data_len, LPDRMETA data_meta);
};

struct conn_http_cmd {
    conn_http_service_t m_service;
    const char * m_path;
    uint32_t m_req_id;
    LPDRMETA m_req_meta;
    uint32_t m_pkg_buf_size;

    TAILQ_ENTRY(conn_http_cmd) m_next_for_service;
};

struct conn_http_service {
    conn_http_svr_t m_svr;
    const char * m_path;
    set_svr_svr_info_t m_dispatch_to;
    conn_http_formator_t m_formator;

    conn_http_cmd_list_t m_cmds;

    TAILQ_ENTRY(conn_http_service) m_next_for_svr;
};

struct conn_http_connection {
    conn_http_svr_t m_svr;
    uint32_t m_id;
    ebb_connection m_ebb_conn;
    conn_http_request_list_t m_requests;

    TAILQ_ENTRY(conn_http_connection) m_next_for_svr;
};

typedef enum conn_http_request_state {
    conn_http_request_init = 1
    , conn_http_request_runing
    , conn_http_request_complete
} conn_http_request_state_t;

struct conn_http_request {
    conn_http_connection_t m_connection;
    uint32_t m_id;
    conn_http_request_state_t m_state;
    ebb_request m_ebb_request;

    ringbuffer_block_t m_req_blk;
    ringbuffer_block_t m_res_blk;

    conn_http_cmd_t m_cmd;

    TAILQ_ENTRY(conn_http_request) m_next_for_connection;
    struct cpe_hash_entry m_hh_for_svr;
};

typedef void (*conn_http_svr_op_t)(conn_http_svr_t svr, dp_req_t pkg_body, dp_req_t pkg_head);

#endif
