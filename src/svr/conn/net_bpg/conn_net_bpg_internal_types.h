#ifndef SVR_CONN_BPG_INTERNAL_TYPES_H
#define SVR_CONN_BPG_INTERNAL_TYPES_H
#include "usf/bpg_pkg/bpg_pkg_types.h"
#include "svr/conn/net_cli/conn_net_cli_types.h"

typedef struct conn_net_bpg_chanel * conn_net_bpg_chanel_t;

struct conn_net_bpg_chanel {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;
    uint32_t m_pkg_max_size;

    bpg_pkg_manage_t m_bpg_pkg_manage;
    conn_net_cli_t m_conn_net_cli;

    conn_net_cli_pkg_t m_conn_head;

    dp_req_t m_incoming_buf;
    cpe_hash_string_t m_incoming_dispatch_to;
    dp_rsp_t m_incoming_recv_at;

    dp_req_t m_outgoing_buf;
    cpe_hash_string_t m_outgoing_dispatch_to;
    dp_rsp_t m_outgoing_recv_at;
};

#endif
