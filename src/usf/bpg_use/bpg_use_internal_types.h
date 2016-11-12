#ifndef USF_BPG_USE_INTERNAL_TYPES_H
#define USF_BPG_USE_INTERNAL_TYPES_H
#include "cpe/utils/hash_string.h"
#include "usf/logic/logic_types.h"
#include "usf/bpg_use/bpg_use_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct bpg_use_sp {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    bpg_pkg_manage_t m_pkg_manage;
    error_monitor_t m_em;

    uint64_t m_client_id;

    dp_req_t m_pkg_buf;
    struct mem_buffer m_data_buf;

    bpg_pkg_dsp_t m_dsp;
    int m_debug;
};

struct bpg_use_pkg_chanel {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    bpg_pkg_manage_t m_pkg_manage;

    dp_rsp_t m_outgoing_recv_at;
    cpe_hash_string_t m_outgoing_send_to;
    dp_req_t m_outgoing_buf;

    dp_rsp_t m_incoming_recv_at;
    bpg_pkg_dsp_t m_incoming_send_to;
    dp_req_t m_incoming_buf;

    int m_debug;
};

#ifdef __cplusplus
}
#endif

#endif
