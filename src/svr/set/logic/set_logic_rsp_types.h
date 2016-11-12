#ifndef USF_BPG_RSP_INTERNAL_TYPES_H
#define USF_BPG_RSP_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "svr/set/logic/set_logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(set_logic_rsp_pkg_builder_list, set_logic_rsp_pkg_builder) * set_logic_rsp_pkg_builder_list_t;
struct set_logic_rsp_queue_info;
struct set_logic_rsp_error_response;

struct set_logic_rsp_manage {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    logic_manage_t m_logic_mgr;
    set_svr_stub_t m_stub;
    set_svr_svr_info_t m_svr_type;
    LPDRMETA m_pkg_meta;
    LPDRMETAENTRY m_pkg_cmd_entry;
    LPDRMETAENTRY m_pkg_data_entry;
    logic_executor_mgr_t m_executor_mgr;
    error_monitor_t m_em;
    uint32_t m_flags;
    dp_mgr_t m_dp;
    dp_req_t m_dp_req_buf;

    struct cpe_hash_table m_rsps;

    size_t m_ctx_capacity;
    set_logic_ctx_init_fun_t m_ctx_init;
    set_logic_ctx_fini_fun_t m_ctx_fini;
    void * m_ctx_ctx;

    dp_req_t m_rsp_buf;

    char * m_queue_attr;

    struct set_logic_rsp_queue_info * m_default_queue_info;
    struct cpe_hash_table m_queue_infos;

    int m_debug;

    const char * m_recv_at;
    cpe_hash_string_t m_commit_to;
};

typedef enum set_logic_rsp_queue_scope {
    set_logic_rsp_queue_scope_global
    , set_logic_rsp_queue_scope_client
} set_logic_rsp_queue_scope_t;

struct set_logic_rsp_queue_info {
    cpe_hash_string_t m_name;
    set_logic_rsp_queue_scope_t m_scope;
    uint32_t m_max_count;
    char m_name_buf[128];
    struct cpe_hash_entry m_hh;
};

struct set_logic_rsp {
    set_logic_rsp_manage_t m_mgr;
    const char * m_name;

    logic_executor_ref_t m_executor_ref;
    uint32_t m_flags;
    struct set_logic_rsp_queue_info *  m_queue_info;
    tl_time_span_t m_timeout_ms;

    struct cpe_hash_entry m_hh;
};

struct set_logic_rsp_error_response {
    uint32_t m_cmd;
    LPDRMETA m_data_meta;
    LPDRMETAENTRY m_errno_entry;
    LPDRMETAENTRY m_req_entry;
};

#ifdef __cplusplus
}
#endif

#endif
