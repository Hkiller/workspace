#ifndef USF_BPG_CLI_INTERNAL_TYPES_H
#define USF_BPG_CLI_INTERNAL_TYPES_H
#include "cpe/dp/dp_types.h"
#include "usf/logic/logic_types.h"
#include "usf/bpg_cli/bpg_cli_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct bpg_cli_proxy {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    logic_manage_t m_logic_mgr;
    bpg_pkg_manage_t m_pkg_manage;
    uint64_t m_client_id;

    dp_rsp_t m_outgoing_recv_at;
    bpg_pkg_dsp_t m_outgoing_send_to;
    size_t m_send_pkg_max_size;
    dp_req_t m_send_pkg_buf;
    struct mem_buffer m_send_data_buf;

    bpg_pkg_dsp_t m_incoming_no_sn_send_to;

    struct mem_buffer m_dump_buf;
    int m_debug;
};

#ifdef __cplusplus
}
#endif

#endif
