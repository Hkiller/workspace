#include <assert.h> 
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_pbuf.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "cpe/aom/aom_obj_hash.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "gd/app/app_log.h"
#include "rank_g_svr_ops.h"
#include "rank_g_svr_index.h"

logic_op_exec_result_t
rank_g_svr_op_update_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg) {
    rank_g_svr_t svr = user_data;
    logic_data_t req_data;
    SVR_RANK_G_REQ_UPDATE * req;
    char * buf;
    int rv;
    ptr_int_t record_idx;
    rank_g_svr_index_t index;
    void const * record;

    req_data = logic_context_data_find(ctx, "svr_rank_g_req_update");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: update: get request fail!", rank_g_svr_name(svr));
        logic_context_errno_set(ctx, SVR_RANK_G_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    buf = rank_g_svr_record_buf(svr, svr->m_record_size);
    assert(buf);

    req = logic_data_data(req_data);

    rv = dr_pbuf_read(buf, svr->m_record_size, req->data, req->data_len, svr->m_record_meta, svr->m_em);
    if (rv < 0) {
        CPE_ERROR(svr->m_em, "%s: request update: read data fail!", rank_g_svr_name(svr));
        logic_context_errno_set(ctx, SVR_RANK_G_ERROR_RECORD_INPUT_DATA);
        return logic_op_exec_result_false;
    }

    TAILQ_FOREACH(index, &svr->m_indexs, m_next) {
        if (!index->m_season_use) continue;
        rv = rank_g_svr_index_season_check_update(index, buf);
        if (rv != 0) {
            logic_context_errno_set(ctx, rv);
            return logic_op_exec_result_false;
        }
    }
    
    if (aom_obj_hash_table_insert_or_update(svr->m_record_hash, buf, &record_idx) != 0) {
        CPE_ERROR(svr->m_em, "%s: request update: create_or_update record fail!", rank_g_svr_name(svr));
        logic_context_errno_set(ctx, SVR_RANK_G_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    record = aom_obj_get(svr->m_record_mgr, record_idx);
    assert(record);

    /* CPE_ERROR(svr->m_em, "        update: count=%d, idx=%d, %s", */
    /*           aom_obj_mgr_allocked_obj_count(svr->m_record_mgr), */
    /*           (int)record_idx, rank_g_svr_record_dump(svr, record)); */
                  
    TAILQ_FOREACH(index, &svr->m_indexs, m_next) {
        rv = rank_g_svr_index_update(index, record, (uint32_t)record_idx);
        if (rv != 0) {
            CPE_ERROR(svr->m_em, "%s: request update: update record %d record fail!", rank_g_svr_name(svr), (int)record_idx);
            logic_context_errno_set(ctx, rv);
            return logic_op_exec_result_false;
        }
    }

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
rank_g_svr_op_update_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg) {
    return logic_op_exec_result_true;
}
