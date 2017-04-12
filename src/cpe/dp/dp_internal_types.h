#ifndef CPE_DP_IMPL_INTERNAL_TYPES_H
#define CPE_DP_IMPL_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/hash.h"
#include "cpe/dp/dp_types.h"
#include "cpe/nm/nm_types.h"
#include "cpe/dr/dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct dp_processing_rsp_buf;
typedef TAILQ_HEAD(dp_processing_rsp_buf_list, dp_processing_rsp_buf) dp_processing_rsp_buf_list_t;

struct dp_mgr {
    mem_allocrator_t m_alloc;
    struct cpe_hash_table m_rsps;
    struct cpe_hash_table m_cmd_2_rsps;
    dp_processing_rsp_buf_list_t m_processiong_rsps;
};

typedef enum dp_key_type {
    dp_key_numeric
    , dp_key_string
} dp_key_type_t;

struct dp_binding {
    struct cpe_hash_entry m_hh;
    dp_rsp_t m_rsp;
    dp_key_type_t m_kt;

    /*make a list to rsp*/
    struct dp_binding * m_rep_binding_next;
    struct dp_binding ** m_rep_binding_pre;

    /*make a list to cmd*/
    struct dp_binding * m_cmd_binding_next;
    struct dp_binding ** m_cmd_binding_pre;
};

struct dp_binding_numeric {
    struct dp_binding m_head;
    int32_t m_value;
};

struct dp_binding_string {
    struct dp_binding m_head;
    const char * m_value;
    uint32_t m_value_len;
};

struct dp_rsp {
    dp_mgr_t m_dp;
    const char * m_name;
    size_t m_name_len;

    dp_rsp_type_t m_type;
    dp_rsp_process_fun_t m_processor;
    void * m_context;

    struct dp_binding * m_bindings;
    struct cpe_hash_entry m_hh;
};

typedef TAILQ_HEAD(dp_req_list, dp_req) dp_req_list_t;

struct dp_req {
    dp_mgr_t m_mgr;
    const char * m_type;
    dp_req_dump_fun_t m_dumper;
    size_t m_capacity;
    dp_req_t m_parent;
    uint8_t m_manage_by_parent;
    LPDRMETA m_data_meta;
    void * m_data;
    size_t m_data_capacity;
    size_t m_data_size;

    dp_req_list_t m_childs;
    TAILQ_ENTRY(dp_req) m_brother;
};

#define PROCESSING_BUF_RSP_COUNT (128)

typedef TAILQ_HEAD(dp_processing_rsp_block_list, dp_processing_rsp_block) dp_processing_rsp_block_list_t;

struct dp_processing_rsp_block {
    size_t m_write_pos;
    size_t m_read_pos;
    dp_rsp_t m_rsps[PROCESSING_BUF_RSP_COUNT];
    TAILQ_ENTRY(dp_processing_rsp_block) m_next;
};

struct dp_processing_rsp_buf {
    mem_allocrator_t m_alloc;
    dp_processing_rsp_block_list_t m_blocks;
    TAILQ_ENTRY(dp_processing_rsp_buf) m_sh_other;
    struct dp_processing_rsp_block m_first_block;
};

#ifdef __cplusplus
}
#endif

#endif
