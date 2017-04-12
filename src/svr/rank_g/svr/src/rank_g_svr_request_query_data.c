#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/aom/aom_obj_hash.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "rank_g_svr_ops.h"

logic_op_exec_result_t
rank_g_svr_op_query_data_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg) {
    rank_g_svr_t svr = user_data;
    logic_data_t req_data;
    SVR_RANK_G_REQ_QUERY_DATA * req;
    logic_data_t res_data;
    SVR_RANK_G_RES_QUERY_DATA * res;
    uint32_t result_capacity;
    uint32_t i;
    char key_buf[svr->m_record_size];

    req_data = logic_context_data_find(ctx, "svr_rank_g_req_query_data");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: query-data: get request fail!", rank_g_svr_name(svr));
        logic_context_errno_set(ctx, SVR_RANK_G_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);
    
    result_capacity = svr->m_record_size * 3 * req->user_id_count;

    res_data = logic_context_data_get_or_create(ctx, svr->m_pkg_meta_res_query_data, sizeof(SVR_RANK_G_RES_QUERY_DATA) +  result_capacity);
    if (res_data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: query-data: create response, data capacity is %d!",
            rank_g_svr_name(svr), result_capacity);
        logic_context_errno_set(ctx, SVR_RANK_G_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    res = logic_data_data(res_data);

    bzero(key_buf, sizeof(key_buf));
    
    for(i = 0; i < req->user_id_count; ++i) {
        void * record;
        int rv;

        if (dr_entry_set_from_uint64(key_buf + svr->m_uin_start_pos, req->user_ids[i], svr->m_uin_entry, svr->m_em) != 0) {
            CPE_ERROR(svr->m_em, "%s: query-data: set role id fail", rank_g_svr_name(svr));
            continue;
        }
        
        record = aom_obj_hash_table_find(svr->m_record_hash, key_buf);
        if (record == NULL) continue;

        rv = dr_pbuf_write_with_size(
            res->data + res->data_len, result_capacity - res->data_len,
            record, svr->m_record_size, svr->m_record_meta,
            svr->m_em);
        if (rv < 0) {
            CPE_ERROR(
                svr->m_em, "%s: query-data: build result: encode data fail, capacity=%d, rv=%d!",
                rank_g_svr_name(svr), result_capacity - res->data_len, rv);
            continue;
        }

        res->data_len += rv;
    }

    return logic_op_exec_result_true;
}
