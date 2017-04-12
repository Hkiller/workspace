#ifndef USF_BPG_BIND_INTERNAL_TYPES_H
#define USF_BPG_BIND_INTERNAL_TYPES_H
#include "cpe/utils/hash.h"
#include "cpe/dp/dp_types.h"
#include "gd/vnet/vnet_types.h"
#include "usf/logic/logic_types.h"
#include "usf/bpg_rsp/bpg_rsp_types.h"
#include "usf/bpg_bind/bpg_bind_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct bpg_bind_manage {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    bpg_pkg_manage_t m_pkg_manage;

    uint32_t m_cmd_kickoff;

    dp_rsp_t m_incoming_recv_at;
    bpg_pkg_dsp_t m_incoming_send_to;

    dp_rsp_t m_outgoing_recv_at;
    bpg_pkg_dsp_t m_outgoing_send_to;

    dp_req_t m_data_pkg;
    vnet_control_pkg_t m_control_pkg;

    struct cpe_hash_table m_cliensts;
    struct cpe_hash_table m_connections;

    int m_debug;
};

struct bpg_bind_binding {
    uint64_t m_client_id;
    uint32_t m_connection_id;

    struct cpe_hash_entry m_hh_client;
    struct cpe_hash_entry m_hh_connection;
};

#ifdef __cplusplus
}
#endif

#endif
