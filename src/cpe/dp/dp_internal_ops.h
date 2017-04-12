#ifndef CPE_DP_IMPL_INTERNAL_OPS_H
#define CPE_DP_IMPL_INTERNAL_OPS_H
#include "dp_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t dp_rsp_hash(const dp_rsp_t rsp);
int dp_rsp_cmp(const dp_rsp_t l, const dp_rsp_t r);
void dp_rsp_free_i(dp_rsp_t rsp);

/*processing buf operations*/
void dp_pbuf_init(dp_mgr_t dm, struct dp_processing_rsp_buf * buf);
void dp_pbuf_fini(dp_mgr_t dm, struct dp_processing_rsp_buf * buf);
void dp_pbuf_add_rsp(struct dp_processing_rsp_buf * buf, dp_rsp_t rsp, error_monitor_t em);
void dp_pbuf_remove_rsp(dp_mgr_t dm, dp_rsp_t rsp);
dp_rsp_t dp_pbuf_retrieve_first(struct dp_processing_rsp_buf * buf);

/*binding operations*/
void dp_binding_free(struct dp_binding * binding);
void dp_binding_free_i(struct dp_binding * binding);

int32_t dp_binding_hash(const struct dp_binding * rsp);
int dp_binding_cmp(const struct dp_binding * l, const struct dp_binding * r);

#ifdef __cplusplus
}
#endif

#endif
