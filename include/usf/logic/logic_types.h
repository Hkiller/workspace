#ifndef USF_LOGIC_TYPES_H
#define USF_LOGIC_TYPES_H
#include "cpe/utils/error.h"
#include "cpe/cfg/cfg_types.h"
#include "cpe/tl/tl_types.h"
#include "gd/app/app_types.h"
#include "cpe/dr/dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INVALID_LOGIC_CONTEXT_ID ((logic_context_id_t)-1)
#define INVALID_LOGIC_REQUIRE_ID ((logic_require_id_t)-1)

typedef uint32_t logic_context_id_t;
typedef uint32_t logic_require_id_t;

typedef enum logic_context_flag {
    logic_context_flag_debug = 1 << 0
    , logic_context_flag_execute_immediately = 1 << 1
} logic_context_flag_t;

typedef enum logic_context_state {
    logic_context_state_init
    , logic_context_state_waiting
    , logic_context_state_idle
    , logic_context_state_error
    , logic_context_state_done
    , logic_context_state_cancel
    , logic_context_state_timeout
} logic_context_state_t;

typedef enum logic_require_state {
    logic_require_state_waiting
    , logic_require_state_canceling
    , logic_require_state_error
    , logic_require_state_done
    , logic_require_state_canceled
    , logic_require_state_timeout
} logic_require_state_t;

typedef enum logic_executor_category {
    logic_executor_category_action
    , logic_executor_category_condition
    , logic_executor_category_decorator
    , logic_executor_category_composite
} logic_executor_category_t;

typedef enum logic_executor_composite_type {
    logic_executor_composite_selector
    , logic_executor_composite_sequence
    , logic_executor_composite_parallel
} logic_executor_composite_type_t;

typedef enum logic_executor_parallel_policy {
    logic_executor_parallel_success_on_all
    , logic_executor_parallel_success_on_one
} logic_executor_parallel_policy_t;

typedef enum logic_executor_decorator_type {
    logic_executor_decorator_protect
    , logic_executor_decorator_not
} logic_executor_decorator_type_t;

typedef struct logic_manage * logic_manage_t;
typedef struct logic_context * logic_context_t;
typedef struct logic_data * logic_data_t;
typedef struct logic_require * logic_require_t;

typedef struct logic_stack * logic_stack_t;
typedef struct logic_stack_node * logic_stack_node_t;

typedef struct logic_queue * logic_queue_t;
typedef struct logic_executor * logic_executor_t;
typedef struct logic_executor_ref * logic_executor_ref_t;
typedef struct logic_executor_mgr * logic_executor_mgr_t;
typedef struct logic_executor_type * logic_executor_type_t;
typedef struct logic_executor_type_group * logic_executor_type_group_t;

typedef enum logic_op_exec_result {
    logic_op_exec_result_true = 1,
    logic_op_exec_result_false = 2,
    logic_op_exec_result_null = 3,
} logic_op_exec_result_t;

typedef logic_op_exec_result_t (*logic_op_fun_t) (logic_context_t ctx, logic_stack_node_t stack_noe, void * user_data, cfg_t cfg);
typedef void (*logic_op_ctx_fini_fun_t)(void*);

typedef logic_executor_t (*logic_executor_create_fun_t) (
    logic_manage_t mgr, const char * name, void * ctx,
    cfg_t args,
    error_monitor_t em);

typedef void (*logic_context_commit_fun_t) (logic_context_t ctx, void * user_data);

typedef struct logic_executor_type_it {
    logic_executor_type_t (*next)(struct logic_executor_type_it * it);
    char m_data[16];
} * logic_executor_type_it_t;

typedef struct logic_require_it {
    logic_require_t (*next)(struct logic_require_it * it);
    char m_data[16];
} * logic_require_it_t;

typedef struct logic_data_it {
    logic_data_t (*next)(struct logic_data_it * it);
    char m_data[16];
} * logic_data_it_t;

#ifdef __cplusplus
}
#endif

#endif
