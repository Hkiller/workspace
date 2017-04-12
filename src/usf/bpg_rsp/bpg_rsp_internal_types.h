#ifndef USF_BPG_RSP_INTERNAL_TYPES_H
#define USF_BPG_RSP_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "gd/dr_store/dr_store_types.h"
#include "usf/bpg_rsp/bpg_rsp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(bpg_rsp_pkg_builder_list, bpg_rsp_pkg_builder) * bpg_rsp_pkg_builder_list_t;
struct bpg_rsp_queue_info;

struct bpg_rsp_manage {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    logic_manage_t m_logic_mgr;
    logic_executor_mgr_t m_executor_mgr;
    bpg_pkg_manage_t m_pkg_manage;
    error_monitor_t m_em;
    uint32_t m_flags;
    dp_mgr_t m_dp;
    dp_req_t m_dp_req_buf;
    const char * m_dispatch_recv_at;
    struct cpe_hash_table m_rsps;

    size_t m_ctx_capacity;
    bpg_logic_ctx_init_fun_t m_ctx_init;
    bpg_logic_ctx_fini_fun_t m_ctx_fini;
    bpg_logic_pkg_init_fun_t m_pkg_init;
    void * m_ctx_ctx;

    size_t m_rsp_max_size;
    dp_req_t m_rsp_buf;

    struct bpg_rsp_pkg_builder_list m_pkg_builders;

    struct bpg_rsp_queue_info * m_default_queue_info;
    struct cpe_hash_table m_queue_infos;

    int m_debug;

    bpg_pkg_dsp_t m_commit_dsp;
    bpg_pkg_dsp_t m_forward_dsp;
};

struct bpg_rsp_copy_info {
    TAILQ_ENTRY(bpg_rsp_copy_info) m_next;
};

typedef TAILQ_HEAD(bpg_rsp_copy_info_list, bpg_rsp_copy_info) * bpg_rsp_copy_info_list_t;

typedef enum bpg_rsp_queue_scope {
    bpg_rsp_queue_scope_global
    , bpg_rsp_queue_scope_client
} bpg_rsp_queue_scope_t;

struct bpg_rsp_queue_info {
    cpe_hash_string_t m_name;
    bpg_rsp_queue_scope_t m_scope;
    uint32_t m_max_count;
    char m_name_buf[128];
    struct cpe_hash_entry m_hh;
};

struct bpg_rsp {
    bpg_rsp_manage_t m_mgr;
    const char * m_name;

    logic_executor_ref_t m_executor_ref;
    uint32_t m_flags;
    struct bpg_rsp_queue_info *  m_queue_info;
    tl_time_span_t m_timeout_ms;

    struct cpe_hash_entry m_hh;

    struct bpg_rsp_copy_info_list m_ctx_to_pdu;
};

struct bpg_rsp_pkg_builder {
    bpg_rsp_manage_t m_mgr;
    bpg_pkg_build_fun_t m_build_fun;
    void * m_build_ctx;

    TAILQ_ENTRY(bpg_rsp_pkg_builder) m_next;
};

#ifdef __cplusplus
}
#endif

#endif
