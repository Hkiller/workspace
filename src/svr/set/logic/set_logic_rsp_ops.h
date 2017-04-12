#ifndef SVR_SET_LOGIC_RSP_OPS_H
#define SVR_SET_LOGIC_RSP_OPS_H
#include "cpe/dp/dp_types.h"
#include "set_logic_rsp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*set_logic_rsp_manage ops*/
dp_req_t set_logic_rsp_manage_rsp_buf(set_logic_rsp_manage_t mgr, size_t data_capacity);
int set_logic_rsp_manage_set_recv_at(set_logic_rsp_manage_t mgr, const char * recv_at);

/*set_logic_rsp ops*/
int set_logic_rsp_execute(dp_req_t req, void * ctx, error_monitor_t em);
void set_logic_rsp_commit(logic_context_t op_context, void * user_data);
uint32_t set_logic_rsp_hash(const struct set_logic_rsp * rsp);
int set_logic_rsp_cmp(const struct set_logic_rsp * l, const struct set_logic_rsp * r);
void set_logic_rsp_free_all(set_logic_rsp_manage_t mgr);

/*set_logic_rsp_queue_info ops*/
const char * set_logic_rsp_queue_name(const struct set_logic_rsp_queue_info * queue);
struct set_logic_rsp_queue_info *
set_logic_rsp_queue_info_create(
    set_logic_rsp_manage_t mgr,
    const char * queue_name,
    set_logic_rsp_queue_scope_t scope,
    uint32_t max_count);
struct set_logic_rsp_queue_info *
set_logic_rsp_queue_info_find(set_logic_rsp_manage_t mgr, cpe_hash_string_t queue_name);

uint32_t set_logic_rsp_queue_info_hash(const struct set_logic_rsp_queue_info * queue);
int set_logic_rsp_queue_info_cmp(const struct set_logic_rsp_queue_info * l, const struct set_logic_rsp_queue_info * r);
void set_logic_rsp_queue_info_free_all(set_logic_rsp_manage_t mgr);

#ifdef __cplusplus
}
#endif

#endif
