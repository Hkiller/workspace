#ifndef CPE_DP_RESPONSER_H
#define CPE_DP_RESPONSER_H
#include "dp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dp_rsp_t dp_rsp_create(dp_mgr_t dp, const char * name);
void dp_rsp_free(dp_rsp_t rsp);

const char * dp_rsp_name(dp_rsp_t rsp);
void dp_rsp_set_type(dp_rsp_t rsp, dp_rsp_type_t type);
dp_rsp_type_t dp_rsp_type(dp_rsp_t rsp);

void dp_rsp_set_processor(dp_rsp_t rsp, dp_rsp_process_fun_t process, void * ctx);
dp_rsp_process_fun_t dp_rsp_processor(dp_rsp_t rsp);
void * dp_rsp_context(dp_rsp_t rsp);

void dp_rsp_bindings(struct dp_binding_it * it, dp_rsp_t rsp);

#ifdef __cplusplus
}
#endif

#endif
