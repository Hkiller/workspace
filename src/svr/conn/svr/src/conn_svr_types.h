#ifndef SVR_CONN_SVR_TYPES_H
#define SVR_CONN_SVR_TYPES_H
#include "ev.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/error.h"
#include "cpe/utils/ringbuffer.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "gd/timer/timer_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "protocol/svr/conn/svr_conn_internal.h"
#include "protocol/svr/conn/svr_conn_meta.h"

typedef struct conn_svr * conn_svr_t;
typedef struct conn_svr_conn * conn_svr_conn_t;
typedef struct conn_svr_backend * conn_svr_backend_t;
typedef TAILQ_HEAD(conn_svr_conn_list, conn_svr_conn) conn_svr_conn_list_t;
 
struct conn_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;
    uint16_t m_conn_svr_type;
    
    struct ev_loop * m_ev_loop;

    uint32_t m_read_block_size;
    uint32_t m_max_pkg_size;

    uint32_t m_conn_timeout_s;
    uint32_t m_conn_max_id;

    gd_timer_id_t m_check_timer_id;

    dp_rsp_t m_ss_request_recv_at;
    dp_rsp_t m_ss_trans_recv_at;
    cpe_hash_string_t m_ss_send_to;

    dp_req_t m_outgoing_pkg;

    int m_fd;
    struct ev_io m_watcher;

    ringbuffer_t m_ringbuf;

    struct cpe_hash_table m_conns_by_conn_id;
    struct cpe_hash_table m_conns_by_user_id;
    conn_svr_conn_list_t m_conns_check;

    struct cpe_hash_table m_backends;
};

struct conn_svr_conn {
    conn_svr_t m_svr;
    int m_fd;
    CONN_SVR_CONN_INFO m_data;
    uint8_t m_auth;

    uint32_t m_last_op_time;
    ringbuffer_block_t m_rb;
    ringbuffer_block_t m_wb;

    struct ev_io m_watcher;
    struct cpe_hash_entry m_hh_for_conn_id;
    struct cpe_hash_entry m_hh_for_user_id;
    TAILQ_ENTRY(conn_svr_conn) m_next_for_check;
};

enum conn_svr_backend_safe_policy {
    conn_svr_backend_any /*直接暴露给客户端使用，主要用于认证服务系列*/
    , conn_svr_backend_auth_success /*认证成功就可以使用*/
    , conn_svr_backend_user_bind /*必须用户绑定后使用*/
};

struct conn_svr_backend {
    conn_svr_t m_svr;
    uint16_t m_svr_type;
    enum conn_svr_backend_safe_policy m_safe_policy;
    LPDRMETA m_pkg_meta;
    size_t m_user_id_start;
    LPDRMETAENTRY m_user_id_entry;

    struct cpe_hash_entry m_hh;
};

typedef void (*conn_svr_op_t)(conn_svr_t svr, dp_req_t pkg);

#endif
