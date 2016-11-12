#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_pbuf.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "rank_g_svr_ops.h"
#include "rank_g_svr_index.h"
#include "rank_g_svr_season_info.h"
#include "rank_g_svr_rank_tree.h"

static int rank_g_svr_op_query_with_data_from_tree(rank_g_svr_t svr, rank_g_svr_index_t index, logic_context_t ctx, SVR_RANK_G_REQ_QUERY_WITH_DATA * req, uint32_t result_capacity);
static SVR_RANK_G_RES_QUERY_WITH_DATA * rank_g_svr_op_query_with_data_create_res(rank_g_svr_t svr, logic_context_t ctx, SVR_RANK_G_REQ_QUERY_WITH_DATA * req, uint32_t result_capacity);

logic_op_exec_result_t
rank_g_svr_op_query_with_data_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg) {
    rank_g_svr_t svr = user_data;
    logic_data_t req_data;
    SVR_RANK_G_REQ_QUERY_WITH_DATA *req;
    rank_g_svr_index_t index;
    int rv;
    uint32_t result_capacity;

    req_data = logic_context_data_find(ctx, "svr_rank_g_req_query_with_data");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: query-with-data: get request fail!", rank_g_svr_name(svr));
        logic_context_errno_set(ctx, SVR_RANK_G_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    index = rank_g_svr_index_find(svr, req->index_id);
    if (index == NULL) {
        logic_context_errno_set(ctx, SVR_RANK_G_ERROR_INDEX_NOT_EXIST);
        return logic_op_exec_result_false;
    }

    if (req->require_count > svr->m_result_count_limit) {
        CPE_ERROR(
            svr->m_em, "%s: query-with-data: request count %d overflow, limit=%d!",
            rank_g_svr_name(svr), req->require_count, svr->m_result_count_limit);
        logic_context_errno_set(ctx, SVR_RANK_G_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    result_capacity = svr->m_record_size * 3 * req->require_count;

    if (index->m_season_use) {
        if (index->m_season_cur == NULL) {
            logic_context_errno_set(ctx, SVR_RANK_G_ERROR_SEASON_NOT_EXIST);
            return logic_op_exec_result_false;
        }
        else if (req->season == 0 || req->season == index->m_season_cur->m_season_id) {
            /*内存查询 */
            if ((rv = rank_g_svr_op_query_with_data_from_tree(svr, index, ctx, req, result_capacity) != 0)) {
                logic_context_errno_set(ctx, rv);
                return logic_op_exec_result_false;
            }
        }
        else {
            /*数据库查询 */
        }
    }
    else {
        if (req->season != 0) {
            CPE_ERROR(
                svr->m_em, "%s: query: index %d not support season, req season = %d",
                rank_g_svr_name(svr), index->m_id, req->season);
            logic_context_errno_set(ctx, SVR_RANK_G_ERROR_INTERNAL);
            return logic_op_exec_result_false;
        }
        else {
            /*内存查询 */
            if ((rv = rank_g_svr_op_query_with_data_from_tree(svr, index, ctx, req, result_capacity) != 0)) {
                logic_context_errno_set(ctx, rv);
                return logic_op_exec_result_false;
            }
        }
    }

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
rank_g_svr_op_query_with_data_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg) {
    return logic_op_exec_result_true;
}

struct rank_g_svr_query_with_data_work_ctx {
    rank_g_svr_t svr;
    rank_g_svr_index_t index;
    SVR_RANK_G_RES_QUERY_WITH_DATA * res;
    uint32_t result_capacity;
};

static int rank_g_svr_build_query_with_data_result(void * ctx, void const * record, rt_node_t node) {
    struct rank_g_svr_query_with_data_work_ctx * work_ctx = ctx;
    rank_g_svr_t svr = work_ctx->svr;
    SVR_RANK_G_RES_QUERY_WITH_DATA * res = work_ctx->res;
    int rv;

    if (res->data_len == 0) { /*第一个 */
        res->result_start = rt_node_rank(work_ctx->index->m_rank_tree, node);
    }
    
    rv = dr_pbuf_write_with_size(
        res->data + res->data_len, work_ctx->result_capacity - res->data_len,
        record, svr->m_record_size, svr->m_record_meta,
        svr->m_em);
    if (rv < 0) {
        CPE_ERROR(
            work_ctx->svr->m_em, "%s: build query result: encode data fail, capacity=%d, rv=%d!",
            rank_g_svr_name(svr), work_ctx->result_capacity - res->data_len, rv);
        return -1;
    }

    res->data_len += rv;

    return 0;
}

static int rank_g_svr_op_query_with_data_from_tree(
    rank_g_svr_t svr, rank_g_svr_index_t index, logic_context_t ctx, SVR_RANK_G_REQ_QUERY_WITH_DATA * req, uint32_t result_capacity)
{
    struct rank_g_svr_query_with_data_work_ctx work_ctx;
    int rv;
    
    work_ctx.svr = svr;
    work_ctx.index = index;
    work_ctx.result_capacity = result_capacity;
    work_ctx.res = rank_g_svr_op_query_with_data_create_res(svr, ctx, req, result_capacity);

    if ((rv = rank_g_svr_index_query(
             svr, index,
             &req->query, req->require_count,
             rank_g_svr_build_query_with_data_result, &work_ctx)))
    {
        CPE_ERROR(svr->m_em, "%s: query-with-data: build response error, rv=%d!", rank_g_svr_name(svr), rv);
        return rv;
    }

    return 0;
}

static SVR_RANK_G_RES_QUERY_WITH_DATA *
rank_g_svr_op_query_with_data_create_res(rank_g_svr_t svr, logic_context_t ctx, SVR_RANK_G_REQ_QUERY_WITH_DATA * req, uint32_t result_capacity) {
    logic_data_t res_data;
    SVR_RANK_G_RES_QUERY_WITH_DATA * res;

    res_data = logic_context_data_get_or_create(ctx, svr->m_pkg_meta_res_query_with_data, sizeof(SVR_RANK_G_RES_QUERY_WITH_DATA) +  result_capacity);
    if (res_data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: query-with-data: create response, data capacity is %d!",
            rank_g_svr_name(svr), result_capacity);
        return NULL;
    }
    
    res = logic_data_data(res_data);
    res->index_id = req->index_id;
    res->query = req->query;
    res->data_len = 0;

    return res;
}
