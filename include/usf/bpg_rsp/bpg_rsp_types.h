#ifndef USF_BPG_RSP_TYPES_H
#define USF_BPG_RSP_TYPES_H
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "cpe/cfg/cfg_types.h"
#include "cpe/dr/dr_types.h"
#include "usf/logic/logic_types.h"
#include "usf/bpg_pkg/bpg_pkg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum bpg_rsp_flag {
    bpg_rsp_flag_debug = 1 << 0
    , bpg_rsp_flag_append_info_manual = 1 << 1
} bpg_rsp_flag_t;

typedef enum bpg_rsp_manage_flag {
    bpg_rsp_manage_flag_sn_use_client = 1 << 0
} bpg_rsp_manage_flag_t;

typedef enum bpg_rsp_manage_dp_scope {
    bpg_rsp_manage_dp_scope_global
    , bpg_rsp_manage_dp_scope_local
} bpg_rsp_manage_dp_scope_t;

typedef uint32_t bpg_req_sn_t;

typedef struct bpg_rsp_manage * bpg_rsp_manage_t;
typedef struct bpg_rsp * bpg_rsp_t;
typedef struct bpg_rsp_carry_info * bpg_rsp_carry_info_t;

typedef struct bpg_rsp_pkg_builder * bpg_rsp_pkg_builder_t;

typedef int (*bpg_logic_ctx_init_fun_t)(logic_context_t context, dp_req_t pkg, void * ctx);
typedef void (*bpg_logic_ctx_fini_fun_t)(logic_context_t context, void * ctx);
typedef int (*bpg_logic_pkg_init_fun_t)(logic_context_t context, dp_req_t pkg, void * ctx);

typedef enum bpg_pkg_build_result {
    bpg_pkg_build_result_success
    , bpg_pkg_build_result_fail
    , bpg_pkg_build_result_unknown
} bpg_pkg_build_result_t;

typedef bpg_pkg_build_result_t (*bpg_pkg_build_fun_t)(dp_req_t pkg, logic_context_t context, const char * data_name, void * ctx);

#ifdef __cplusplus
}
#endif

#endif
