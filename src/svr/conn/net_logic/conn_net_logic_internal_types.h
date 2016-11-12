#ifndef SVR_CONN_LOGIC_INTERNAL_TYPES_H
#define SVR_CONN_LOGIC_INTERNAL_TYPES_H
#include "usf/logic_use/logic_use_types.h"
#include "svr/conn/net_logic/conn_net_logic_types.h"

struct conn_net_logic_sp {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    conn_net_cli_t m_cli;
    int m_debug;

    LPDRMETA m_pkg_info_meta;

    logic_require_queue_t m_require_queue;
    conn_net_cli_pkg_t m_outgoing_pkg;
    dp_req_t m_outgoing_body;

    cpe_hash_string_t m_outgoing_dispatch_to;
    dp_rsp_t m_incoming_recv_at;

};

#endif
