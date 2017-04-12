#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_data.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "rank_g_svr_ops.h"
#include "rank_g_svr_index.h"
#include "rank_g_svr_rank_tree.h"
#include "rank_g_svr_season_info.h"

static int rank_g_svr_op_query_from_tree(rank_g_svr_t svr, rank_g_svr_index_t index, logic_context_t ctx, SVR_RANK_G_REQ_QUERY * req);
static SVR_RANK_G_RES_QUERY * rank_g_svr_op_query_create_res(rank_g_svr_t svr, logic_context_t ctx, rank_g_svr_index_t index, SVR_RANK_G_REQ_QUERY * req);

logic_op_exec_result_t
rank_g_svr_op_query_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg) {
    rank_g_svr_t svr = user_data;
    logic_data_t req_data;
    SVR_RANK_G_REQ_QUERY * req;
    rank_g_svr_index_t index;
    int rv;

    req_data = logic_context_data_find(ctx, "svr_rank_g_req_query");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: query: get request fail!", rank_g_svr_name(svr));
        logic_context_errno_set(ctx, SVR_RANK_G_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    index = rank_g_svr_index_find(svr, req->index_id);
    if (index == NULL) {
        logic_context_errno_set(ctx, SVR_RANK_G_ERROR_INDEX_NOT_EXIST);
        return logic_op_exec_result_true;
    }

    if (req->require_count > svr->m_result_count_limit) {
        CPE_ERROR(
            svr->m_em, "%s: query: request count overflow, require_count=%d, limit=%d!",
            rank_g_svr_name(svr), req->require_count, svr->m_result_count_limit);
        logic_context_errno_set(ctx, SVR_RANK_G_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (index->m_season_use) {
        if (index->m_season_cur == NULL) {
            logic_context_errno_set(ctx, SVR_RANK_G_ERROR_SEASON_NOT_EXIST);
            return logic_op_exec_result_false;
        }
        else if (req->season == 0 || req->season == index->m_season_cur->m_season_id) {
            /*内存查询 */
            if ((rv = rank_g_svr_op_query_from_tree(svr, index, ctx, req)) != 0) {
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
            if ((rv = rank_g_svr_op_query_from_tree(svr, index, ctx, req)) != 0) {
                logic_context_errno_set(ctx, rv);
                return logic_op_exec_result_false;
            }
        }
    }
    
    return logic_op_exec_result_true;
}

logic_op_exec_result_t
rank_g_svr_op_query_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg) {
    return logic_op_exec_result_true;
}

struct rank_g_svr_query_work_ctx {
    rank_g_svr_t svr;
    rank_g_svr_index_t index;
    SVR_RANK_G_RES_QUERY * res;
};

static int rank_g_svr_build_query_result(void * ctx, void const * record, rt_node_t node) {
    struct rank_g_svr_query_work_ctx * work_ctx = ctx;
    SVR_RANK_G_RES_QUERY * res = work_ctx->res;
    uint64_t user_id;

    if (dr_entry_try_read_uint64(&user_id, ((const char *)record) + work_ctx->svr->m_uin_start_pos, work_ctx->svr->m_uin_entry, work_ctx->svr->m_em) != 0) {
        CPE_ERROR(work_ctx->svr->m_em, "%s: build query result: read usr id fail!", rank_g_svr_name(work_ctx->svr));
        return -1;
    }

    if (res->user_id_count == 0) {
        res->result_start = rt_node_rank(work_ctx->index->m_rank_tree, node);
    }
    
    res->user_ids[res->user_id_count++] = user_id;

    return 0;
}

static int rank_g_svr_op_query_from_tree(rank_g_svr_t svr, rank_g_svr_index_t index, logic_context_t ctx, SVR_RANK_G_REQ_QUERY * req) {
    struct rank_g_svr_query_work_ctx work_ctx;
    int rv;
    
    work_ctx.svr = svr;
    work_ctx.index = index;
    work_ctx.res = rank_g_svr_op_query_create_res(svr, ctx, index, req);
    if (work_ctx.res == NULL) return -1;

    if ((rv = rank_g_svr_index_query(
             svr, index, 
             &req->query, req->require_count,
             rank_g_svr_build_query_result, &work_ctx)))
    {
        CPE_ERROR(svr->m_em, "%s: query: build response error, rv=%d!", rank_g_svr_name(svr), rv);
        return rv;
    }

    return 0;
}

static SVR_RANK_G_RES_QUERY *
rank_g_svr_op_query_create_res(rank_g_svr_t svr, logic_context_t ctx, rank_g_svr_index_t index, SVR_RANK_G_REQ_QUERY * req) {
    uint32_t res_data_capacity;
    logic_data_t res_data;
    SVR_RANK_G_RES_QUERY * res;

    res_data_capacity = sizeof(SVR_RANK_G_RES_QUERY) + sizeof(uint64_t) * req->require_count;
    res_data = logic_context_data_get_or_create(ctx, svr->m_pkg_meta_res_query, res_data_capacity);
    if (res_data == NULL) {
        CPE_ERROR(svr->m_em, "%s: query: create res fail, capacity %d", rank_g_svr_name(svr), res_data_capacity);
        return NULL;
    }

    res = logic_data_data(res_data);
    res->index_id = req->index_id;
    res->query = req->query;
    res->total_count = rt_size(index->m_rank_tree);
    res->user_id_count = 0;

    return res;
}
