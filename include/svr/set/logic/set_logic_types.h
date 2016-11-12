#ifndef SVR_SET_LOGIC_TYPES_H
#define SVR_SET_LOGIC_TYPES_H
#include "cpe/pal/pal_types.h"
#include "usf/logic/logic_types.h"
#include "svr/set/stub/set_svr_stub_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct set_logic_sp * set_logic_sp_t;


typedef enum set_logic_rsp_flag {
    set_logic_rsp_flag_debug = 1 << 0
    , set_logic_rsp_flag_append_info_manual = 1 << 1
} set_logic_rsp_flag_t;

typedef enum set_logic_rsp_manage_flag {
    set_logic_rsp_manage_flag_sn_use_client = 1 << 0
} set_logic_rsp_manage_flag_t;

typedef enum set_logic_rsp_manage_dp_scope {
    set_logic_rsp_manage_dp_scope_global
    , set_logic_rsp_manage_dp_scope_local
} set_logic_rsp_manage_dp_scope_t;

typedef struct set_logic_rsp_manage * set_logic_rsp_manage_t;
typedef struct set_logic_rsp * set_logic_rsp_t;
typedef struct set_logic_rsp_carry_info * set_logic_rsp_carry_info_t;

typedef int (*set_logic_ctx_init_fun_t)(logic_context_t context, dp_req_t pkg, void * ctx);
typedef void (*set_logic_ctx_fini_fun_t)(logic_context_t context, void * ctx);

typedef enum set_pkg_build_result {
    set_pkg_build_result_success
    , set_pkg_build_result_fail
    , set_pkg_build_result_unknown
} set_pkg_build_result_t;

typedef set_pkg_build_result_t (*set_pkg_build_fun_t)(dp_req_t pkg, logic_context_t context, const char * data_name, void * ctx);

#ifdef __cplusplus
}
#endif

#endif
