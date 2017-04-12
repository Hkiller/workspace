#ifndef CPE_DP_MANAGE_H
#define CPE_DP_MANAGE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/error.h"
#include "cpe/cfg/cfg_types.h"
#include "cpe/tl/tl_types.h"
#include "dp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dp_mgr_t dp_mgr_create(mem_allocrator_t alloc);
void dp_mgr_free(dp_mgr_t dp);

/*bind command to rsp*/
int dp_mgr_unbind_numeric(dp_mgr_t dp, int32_t cmd);
int dp_mgr_unbind_string(dp_mgr_t dp, const char * cmd);

int dp_rsp_bind_numeric(dp_rsp_t rsp, int32_t cmd, error_monitor_t em);
int dp_rsp_bind_string(dp_rsp_t rsp, const char * cmd, error_monitor_t em);
int dp_rsp_bind_by_cfg(dp_rsp_t dp_rsp, cfg_t cfg_respons, error_monitor_t em);
int dp_rsp_bind_by_cfg_ex(
    dp_rsp_t dp_rsp, cfg_t cfg_respons,
    dp_str_cmd_cvt_t cmd_cvt, void * cmd_cvt_ctx,
    error_monitor_t em);

int dp_rsp_unbind_numeric(dp_rsp_t dp, int32_t cmd);
int dp_rsp_unbind_string(dp_rsp_t dp, const char * cmd);

/*rsp find operations*/
dp_rsp_t dp_rsp_find_by_name(dp_mgr_t dp, const char * name);
dp_rsp_t dp_rsp_find_first_by_numeric(dp_mgr_t dp, int32_t cmd);
dp_rsp_t dp_rsp_find_first_by_string(dp_mgr_t dp, const char * cmd);
void dp_rsp_find_by_numeric(dp_rsp_it_t it, dp_mgr_t dp, int32_t cmd);
void dp_rsp_find_by_string(dp_rsp_it_t it, dp_mgr_t dp, const char * cmd);

int dp_dispatch_by_string(cpe_hash_string_t cmd, dp_mgr_t dm , dp_req_t req, error_monitor_t em);
int dp_dispatch_by_numeric(int32_t cmd, dp_mgr_t dm , dp_req_t req, error_monitor_t em);
int dp_dispatch_by_name(const char * name, dp_mgr_t dm , dp_req_t req, error_monitor_t em);

/*iterator operations*/
#define dp_rsp_next(it) (it)->m_next_fun((it))

#ifdef __cplusplus
}
#endif

#endif
