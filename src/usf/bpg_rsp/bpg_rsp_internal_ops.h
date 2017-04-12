#ifndef USF_BPG_RSP_INTERNAL_OPS_H
#define USF_BPG_RSP_INTERNAL_OPS_H
#include "cpe/dp/dp_types.h"
#include "bpg_rsp_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*bpg_rsp_manage ops*/
dp_req_t bpg_rsp_manage_rsp_buf(bpg_rsp_manage_t mgr);

/*bpg_rsp ops*/
int bpg_rsp_execute(dp_req_t req, void * ctx, error_monitor_t em);
void bpg_rsp_commit(logic_context_t op_context, void * user_data);
uint32_t bpg_rsp_hash(const struct bpg_rsp * rsp);
int bpg_rsp_cmp(const struct bpg_rsp * l, const struct bpg_rsp * r);
void bpg_rsp_free_all(bpg_rsp_manage_t mgr);

/*bpg_rsp_copy_info ops*/
struct bpg_rsp_copy_info *
bpg_rsp_copy_info_create(bpg_rsp_t rsp, const char * data_name);
void bpg_rsp_copy_info_free(bpg_rsp_t rsp, struct bpg_rsp_copy_info * copy_info);
void bpg_rsp_copy_info_clear(bpg_rsp_t rsp);

const char * bpg_rsp_copy_info_data(struct bpg_rsp_copy_info * copy_info);

/*bpg_rsp_queue_info ops*/
const char * bpg_rsp_queue_name(const struct bpg_rsp_queue_info * queue);
struct bpg_rsp_queue_info *
bpg_rsp_queue_info_create(
    bpg_rsp_manage_t mgr,
    const char * queue_name,
    bpg_rsp_queue_scope_t scope,
    uint32_t max_count);
struct bpg_rsp_queue_info *
bpg_rsp_queue_info_find(bpg_rsp_manage_t mgr, cpe_hash_string_t queue_name);

uint32_t bpg_rsp_queue_info_hash(const struct bpg_rsp_queue_info * queue);
int bpg_rsp_queue_info_cmp(const struct bpg_rsp_queue_info * l, const struct bpg_rsp_queue_info * r);
void bpg_rsp_queue_info_free_all(bpg_rsp_manage_t mgr);

/*other*/
#define bpg_rsp_pkg_need_debug_detail(_rsp, _pkg) ((_rsp)->m_mgr->m_debug >= 2 || (_pkg && bpg_pkg_debug_level(_pkg) >= bpg_pkg_debug_progress))

#ifdef __cplusplus
}
#endif

#endif
