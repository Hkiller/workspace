#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_data.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/aom/aom_obj_hash.h"
#include "rank_g_svr_ops.h"
#include "rank_g_svr_index.h"

logic_op_exec_result_t
rank_g_svr_op_remove_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg) {
    rank_g_svr_t svr = user_data;
    logic_data_t req_data;
    SVR_RANK_G_REQ_REMOVE * req;
    char buf[svr->m_record_size];
    ptr_int_t record_idx;
    void * record;
    rank_g_svr_index_t index;

    req_data = logic_context_data_find(ctx, "svr_rank_g_req_remove");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: remove: get request fail!", rank_g_svr_name(svr));
        logic_context_errno_set(ctx, SVR_RANK_G_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);
    
    if (dr_entry_set_from_uint64(buf + svr->m_uin_start_pos, req->user_id, svr->m_uin_entry, svr->m_em) != 0) {
        CPE_ERROR(svr->m_em, "%s: remove: set uin to data data fail!", rank_g_svr_name(svr));
        logic_context_errno_set(ctx, SVR_RANK_G_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    record = aom_obj_hash_table_find(svr->m_record_hash, buf);
    if (record == NULL) {
        CPE_INFO(svr->m_em, "%s: remove: record of "FMT_UINT64_T" not exist!", rank_g_svr_name(svr), req->user_id);
        logic_context_errno_set(ctx, SVR_RANK_G_ERROR_RECORD_NOT_EXIST);
        return logic_op_exec_result_false;
    }
    record_idx = aom_obj_index(svr->m_record_mgr, record);

    TAILQ_FOREACH(index, &svr->m_indexs, m_next) {
        rank_g_svr_index_remove(index, (uint32_t)record_idx);
    }

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
rank_g_svr_op_remove_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg) {
    return logic_op_exec_result_true;
}
