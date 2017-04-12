#ifndef USF_BPG_NET_INTERNAL_TYPES_H
#define USF_BPG_NET_INTERNAL_TYPES_H
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "cpe/net/net_types.h"
#include "usf/bpg_net/bpg_net_types.h"
#include "usf/logic/logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct bpg_net_agent {
    gd_app_context_t m_app;
    bpg_pkg_manage_t m_pkg_manage;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    net_listener_t m_listener;

    size_t m_req_max_size;
    dp_req_t m_req_buf;
    struct mem_buffer m_rsp_buf;

    size_t m_read_chanel_size;
    size_t m_write_chanel_size;

    tl_time_span_t m_conn_timeout;

    cpe_hash_string_t m_dispatch_to;

    dp_rsp_t m_reply_rsp;

    struct mem_buffer m_dump_buffer;

    int m_debug;
};

struct bpg_net_client {
    gd_app_context_t m_app;
    bpg_pkg_manage_t m_pkg_manage;
    logic_manage_t m_logic_mgr;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    net_connector_t m_connector;

    size_t m_req_max_size;
    dp_req_t m_req_buf;
    struct mem_buffer m_send_encode_buf;

    uint32_t m_runing_require_capacity;
    uint32_t m_runing_require_count;
    uint32_t m_runing_require_op_count;
    uint32_t m_runing_require_check_span;
    logic_require_id_t * m_runing_requires;

    bpg_pkg_dsp_t m_rsp_dsp;

    dp_rsp_t m_send_rsp;

    struct mem_buffer m_dump_buffer;

    int8_t m_debug;
};

#ifdef __cplusplus
}
#endif

#endif
