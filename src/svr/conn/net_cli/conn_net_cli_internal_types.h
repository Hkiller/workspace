#ifndef CONN_CLI_INTERNAL_TYPES_H
#define CONN_CLI_INTERNAL_TYPES_H
#include "ev.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/ringbuffer.h"
#include "cpe/fsm/fsm_def.h"
#include "cpe/fsm/fsm_ins.h"
#include "gd/timer/timer_manage.h"
#include "svr/conn/net_cli/conn_net_cli_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct conn_net_cli_monitor * conn_net_cli_monitor_t;
typedef struct conn_net_cli_cmd_info * conn_net_cli_cmd_info_t;
typedef TAILQ_HEAD(conn_net_cli_monitor_list, conn_net_cli_monitor) conn_net_cli_monitor_list_t;

struct conn_net_cli {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int8_t m_debug;
    char m_ip[16];
    short m_port;
    uint8_t m_auto_reconnect;

    uint32_t m_decode_block_size;
    uint32_t m_read_block_size;
    uint32_t m_reconnect_span_ms;
    size_t m_max_pkg_size;
    struct ev_loop * m_ev_loop;

    fsm_def_machine_t m_fsm_def;
    struct fsm_machine m_fsm;
    gd_timer_id_t m_fsm_timer_id;

    int m_fd;
    struct ev_io m_watcher;

    ringbuffer_t m_ringbuf;
    ringbuffer_block_t m_rb;
    ringbuffer_block_t m_wb;
    ringbuffer_block_t m_tb;

    conn_net_cli_pkg_t m_incoming_pkg;
    dp_req_t m_incoming_body;

    dp_req_t m_outgoing_body;

    struct mem_buffer m_dump_buffer;

    struct cpe_hash_table m_svrs;
    conn_net_cli_monitor_list_t m_monitors;
};

struct conn_net_cli_monitor {
    conn_net_cli_t m_cli;
    conn_net_cli_state_process_fun_t m_process_fun;
    void * m_process_ctx;
    TAILQ_ENTRY(conn_net_cli_monitor) m_next;
};

struct conn_net_cli_cmd_info {
    const char * m_meta_name;
    LPDRMETAENTRY m_entry;
    struct cpe_hash_entry m_hh;
};

struct conn_net_cli_svr_stub {
    conn_net_cli_t m_cli;
    uint16_t m_svr_type_id;
    char * m_svr_type_name;

    LPDRMETA m_pkg_meta;
    LPDRMETAENTRY m_pkg_cmd_entry;
    LPDRMETAENTRY m_pkg_data_entry;

    LPDRMETA m_error_pkg_meta;
    uint32_t m_error_pkg_cmd;
    LPDRMETAENTRY m_error_pkg_error_entry;

    cpe_hash_string_t m_response_dispatch_to;
    cpe_hash_string_t m_notify_dispatch_to;
    dp_rsp_t m_outgoing_recv_at;

    struct cpe_hash_entry m_hh;
    struct cpe_hash_table m_cmds;
};

enum conn_net_cli_fsm_evt_type {
    conn_net_cli_fsm_evt_start
    , conn_net_cli_fsm_evt_stop
    , conn_net_cli_fsm_evt_timeout
    , conn_net_cli_fsm_evt_connected
    , conn_net_cli_fsm_evt_disconnected
};

struct conn_net_cli_fsm_evt {
    enum conn_net_cli_fsm_evt_type m_type;
};

struct conn_net_cli_pkg {
    conn_net_cli_t m_cli;
    dp_req_t m_dp_req;
    uint16_t m_svr_type;
    int8_t m_result;
    int8_t m_flags;
    uint32_t m_sn;
};

#ifdef __cplusplus
}
#endif

#endif
