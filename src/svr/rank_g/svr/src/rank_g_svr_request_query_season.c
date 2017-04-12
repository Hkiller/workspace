#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_data.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "rank_g_svr_ops.h"
#include "rank_g_svr_index.h"
#include "rank_g_svr_season_info.h"

logic_op_exec_result_t
rank_g_svr_op_query_season_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg) {
    rank_g_svr_t svr = user_data;
    uint32_t res_data_capacity;
    logic_data_t res_data;
    SVR_RANK_G_RES_QUERY_SEASON * res;
    rank_g_svr_index_t index;

    res_data_capacity = sizeof(SVR_RANK_G_RES_QUERY_SEASON) + sizeof(SVR_RANK_G_SEASON) * svr->m_index_count;
    res_data = logic_context_data_get_or_create(ctx, svr->m_pkg_meta_res_query_season, res_data_capacity);
    if (res_data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: query: create res fail, capacity %d",
            rank_g_svr_name(svr), res_data_capacity);
        logic_context_errno_set(ctx, SVR_RANK_G_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    res = logic_data_data(res_data);

    res->season_count = 0;

    TAILQ_FOREACH(index, &svr->m_indexs, m_next) {
        SVR_RANK_G_SEASON * res_season;
        
        if (!index->m_season_use) continue;

        res_season = &res->seasons[res->season_count++];

        res_season->index_id = index->m_id;

        if (index->m_season_cur) {
            res_season->season = index->m_season_cur->m_season_id;
            res_season->begin_time = index->m_season_cur->m_begin_time;
            res_season->end_time = index->m_season_cur->m_end_time;
        }
        else {
            res_season->season = 0;
            res_season->begin_time = 0;
            res_season->end_time = 0;
        }
    }

    return logic_op_exec_result_true;
}
