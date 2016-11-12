#ifndef USF_LOGIC_INTERNAL_TYPES_H
#define USF_LOGIC_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "gd/timer/timer_types.h"
#include "usf/logic/logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(logic_require_list, logic_require) logic_require_list_t;
typedef TAILQ_HEAD(logic_data_list, logic_data) logic_data_list_t;
typedef TAILQ_HEAD(logic_executor_list, logic_executor) logic_executor_list_t;
typedef TAILQ_HEAD(logic_context_list, logic_context) logic_context_list_t;

struct logic_manage {
    mem_allocrator_t m_alloc;
    gd_app_context_t m_app;
    gd_timer_mgr_t m_timer_mgr;
    tl_time_span_t m_require_timout_ms;
    tl_time_span_t m_context_timout_ms;

    int m_debug;

    uint32_t m_context_id;
    uint32_t m_require_id;

    struct cpe_hash_table m_contexts;
    struct cpe_hash_table m_requires;
    struct cpe_hash_table m_queues;
    struct cpe_hash_table m_datas;

    uint32_t m_waiting_count;
    logic_context_list_t m_waiting_contexts;
    uint32_t m_pending_count;
    logic_context_list_t m_pending_contexts;
};

struct logic_stack_node {
#ifdef USF_LOGIC_DEBUG_MEMORY
    unsigned char m_protect_1[USF_LOGIC_DEBUG_MEMORY];
#endif

    logic_executor_t m_executr;
    logic_context_t m_context;
    logic_data_list_t m_datas;
    uint32_t m_require_waiting_count;
    logic_require_list_t m_requires;
    logic_op_exec_result_t m_rv;

#ifdef USF_LOGIC_DEBUG_MEMORY
    unsigned char m_protect_2[USF_LOGIC_DEBUG_MEMORY];
#endif

};

struct logic_stack {
    struct logic_stack_node m_inline_items[8];
    struct logic_stack_node * m_extern_items;
    int32_t m_extern_items_capacity;
    int32_t m_item_pos; 
};

enum logic_context_queue_state{
    logic_context_queue_none
    , logic_context_queue_waiting
    , logic_context_queue_pending
};

struct logic_context {
    logic_manage_t m_mgr;
    logic_context_id_t m_id;
    logic_context_state_t m_state;
    gd_timer_id_t m_timer_id;

    logic_context_commit_fun_t m_commit_op;
    void * m_commit_ctx;
    size_t m_capacity;
    uint32_t m_flags;
    uint8_t m_runing;
    uint8_t m_deleting;

    logic_data_list_t m_datas;
    uint32_t m_require_waiting_count;
    logic_require_list_t m_requires;

    struct logic_stack m_stack;
    int32_t m_errno;

    logic_queue_t m_logic_queue;
    TAILQ_ENTRY(logic_context) m_next_logic_queue;
    
    struct cpe_hash_entry m_hh;

    enum logic_context_queue_state m_queue_state;
    TAILQ_ENTRY(logic_context) m_next;
};

struct logic_queue {
    logic_manage_t m_mgr;
    cpe_hash_string_t m_name;
    logic_context_list_t m_contexts;
    uint32_t m_count;
    uint32_t m_max_count;
    struct cpe_hash_entry m_hh;
};

struct logic_require {
#ifdef USF_LOGIC_DEBUG_MEMORY
    unsigned char m_protect_1[USF_LOGIC_DEBUG_MEMORY];
#endif

    logic_context_t m_context;
    logic_stack_node_t m_stack;
    logic_require_id_t m_id;
    logic_require_state_t m_state;
    char * m_name;
    int32_t m_error;
    logic_data_list_t m_datas;
    gd_timer_id_t m_timer_id;

    TAILQ_ENTRY(logic_require) m_next_for_context;
    TAILQ_ENTRY(logic_require) m_next_for_stack;

    struct cpe_hash_entry m_hh;

#ifdef USF_LOGIC_DEBUG_MEMORY
    unsigned char m_protect_2[USF_LOGIC_DEBUG_MEMORY];
#endif
};

enum logic_data_owner_type {
    logic_data_owner_context
    , logic_data_owner_stack
    , logic_data_owner_require
};

union logic_data_owner_data {
    logic_context_t m_context;
    logic_require_t m_require;
    logic_stack_node_t m_stack;
};

struct logic_data {
#ifdef USF_LOGIC_DEBUG_MEMORY
    unsigned char m_protect_1[USF_LOGIC_DEBUG_MEMORY];
#endif

    enum logic_data_owner_type m_owner_type;
    union logic_data_owner_data m_owner_data;
    const char * m_name;
    LPDRMETA m_meta;
    size_t m_capacity;

    TAILQ_ENTRY(logic_data) m_next;
    struct cpe_hash_entry m_hh;

#ifdef USF_LOGIC_DEBUG_MEMORY
    unsigned char m_protect_2[USF_LOGIC_DEBUG_MEMORY];
#endif
};

#define LOGIC_EXECUTOR_COMMON                   \
    logic_manage_t m_mgr;                       \
    logic_executor_category_t m_category;       \
    TAILQ_ENTRY(logic_executor) m_next

struct logic_executor {
    LOGIC_EXECUTOR_COMMON;
};

struct logic_executor_action {
    LOGIC_EXECUTOR_COMMON;
    logic_executor_type_t m_type;
    cfg_t m_args;
};

struct logic_executor_decorator {
    LOGIC_EXECUTOR_COMMON;
    logic_executor_decorator_type_t m_decorator_type;
    logic_executor_t m_inner;
};

union logic_executor_composite_arg {
    logic_executor_parallel_policy_t m_parallel_policy;
};

struct logic_executor_composite {
    LOGIC_EXECUTOR_COMMON;
    logic_executor_composite_type_t m_composite_type;
    union logic_executor_composite_arg m_args;
    logic_executor_list_t m_members;
};

struct logic_executor_condition {
    LOGIC_EXECUTOR_COMMON;
    logic_executor_t m_if;
    logic_executor_t m_do;
    logic_executor_t m_else;
};

struct logic_executor_type {
    logic_executor_type_group_t m_group;
    char * m_name;
    void * m_op;
    void * m_ctx;
    logic_op_ctx_fini_fun_t m_ctx_fini;

    struct cpe_hash_entry m_hh;
};

struct logic_executor_type_group {
    mem_allocrator_t m_alloc;
    gd_app_context_t m_app;
    struct cpe_hash_table m_types;
};

struct logic_executor_ref {
    logic_executor_mgr_t m_mgr;
    uint16_t m_ref_count;
    const char * m_name;
    logic_executor_t m_executor;

    struct cpe_hash_entry m_hh;
};

struct logic_executor_mgr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;

    struct cpe_hash_table m_executor_refs;
};

#ifdef __cplusplus
}
#endif

#endif
