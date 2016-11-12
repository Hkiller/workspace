#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_stack.h"
#include "usf/logic/logic_context.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "svr/set/logic/set_logic_rsp_manage.h"
#include "rank_g_svr_ops.h"
#include "rank_g_svr_index.h"
#include "rank_g_svr_rank_tree.h"
#include "rank_g_svr_season_info.h"
#include "rank_g_svr_db_record.h"
#include "rank_g_svr_db_ops.h"

static int rank_g_svr_op_change_season_save_record(
    rank_g_svr_t svr, rank_g_svr_index_t index, logic_stack_node_t stack, rt_node_t node);
static int rank_g_svr_op_change_season_on_save_record_result(
    rank_g_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    SVR_RANK_G_SEASON_SAVING * req, rank_g_svr_index_t index);
static int rank_g_svr_op_change_season_on_save_role_to_rank_result(
    rank_g_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    SVR_RANK_G_SEASON_SAVING * req, rank_g_svr_index_t index);

static SVR_RANK_G_SEASON_SAVING * rank_g_svr_op_change_season_save_req(rank_g_svr_t svr, logic_context_t ctx);
static void rank_g_svr_op_change_season_save_done(rank_g_svr_t svr, rank_g_svr_index_t index);
static void rank_g_svr_op_change_season_save_error(rank_g_svr_t svr, rank_g_svr_index_t index, logic_stack_node_t stack);
    
logic_op_exec_result_t
rank_g_svr_op_change_season_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg) {
    rank_g_svr_t svr = user_data;
    SVR_RANK_G_SEASON_SAVING * req;
    rank_g_svr_index_t index;
    rt_node_t node;

    assert(svr->m_saving_error_time == 0);
    
    req = rank_g_svr_op_change_season_save_req(svr, ctx);
    assert(req);
    assert(req->processing_count == 0);
    
    index = rank_g_svr_index_find(svr, req->index_id);
    assert(index);
    
    if (req->rank_begin == req->rank_end) {
        rank_g_svr_op_change_season_save_done(svr, index);
        return logic_op_exec_result_true;
    }

    node = rt_find_by_rank(index->m_rank_tree, req->rank_begin);
    if (node == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: change season: index %d: first rank node %d not exist",
            rank_g_svr_name(svr), index->m_id, req->rank_begin);
        rank_g_svr_op_change_season_save_error(svr, index, stack);
        return logic_op_exec_result_false;
    }

    if (rank_g_svr_op_change_season_save_record(svr, index, stack, node) != 0) {
        rank_g_svr_op_change_season_save_error(svr, index, stack);
        return logic_op_exec_result_false;
    }
    req->processing_count++;

    for(req->rank_begin++; req->rank_begin < req->rank_end; ++req->rank_begin) {
        node = rt_next(index->m_rank_tree, node);
        if (node == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: change season: index %d: next rank node fail, rank=%d",
                rank_g_svr_name(svr), index->m_id, req->rank_begin);
            rank_g_svr_op_change_season_save_error(svr, index, stack);
            return logic_op_exec_result_false;
        }

        if (rank_g_svr_op_change_season_save_record(svr, index, stack, node) != 0) {
            rank_g_svr_op_change_season_save_error(svr, index, stack);
            return logic_op_exec_result_false;
        }
        req->processing_count++;

        if (req->processing_count >= svr->m_saving_max_pkg_count) break; /*最大并行一百个请求 */
    }

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
rank_g_svr_op_change_season_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg) {
    rank_g_svr_t svr = user_data;
    SVR_RANK_G_SEASON_SAVING * req;
    rank_g_svr_index_t index;
    const char * require_name = logic_require_name(require);
    int rv;

    req = rank_g_svr_op_change_season_save_req(svr, ctx);
    assert(req);
    
    index = rank_g_svr_index_find(svr, req->index_id);
    assert(index);

    /*检查数据库返回结果 */
    if (logic_require_state(require) != logic_require_state_done) {
        CPE_ERROR(
            svr->m_em, "%s: change season: index %d: db request %s error, state=%s, error=%d!",
            rank_g_svr_name(svr), index->m_id, require_name,
            logic_require_state_name(logic_require_state(require)),
            logic_require_error(require));
        rv = -1;
    }
    else {
        if (cpe_str_start_with(require_name, "record_")) {
            rv = rank_g_svr_op_change_season_on_save_record_result(svr, ctx, stack, require, req, index);
        }
        else if (cpe_str_start_with(require_name, "role_to_rank_")) {
            rv = rank_g_svr_op_change_season_on_save_role_to_rank_result(svr, ctx, stack, require, req, index);
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: change season: index %d: unknown require %s",
                rank_g_svr_name(svr), index->m_id, require_name);
             rv = -1;
        }
    }
    
    if (rv != 0) {
        rank_g_svr_op_change_season_save_error(svr, index, stack);
        return logic_op_exec_result_false;
    }
    else {
        return logic_op_exec_result_true;
    }
}

static int rank_g_svr_op_change_season_on_save_record_result(
    rank_g_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    SVR_RANK_G_SEASON_SAVING * req, rank_g_svr_index_t index)
{
    SVR_RANK_G_SEASON_ROLE_TO_RANK * role_to_rank;
    char require_name[64];
    logic_require_t new_require;

    role_to_rank = logic_data_data(logic_require_data_find(require, dr_meta_name(svr->m_meta_season_role_to_rank)));
    
    snprintf(require_name, sizeof(require_name), "role_to_rank_%d", role_to_rank->rank);
    new_require = logic_require_create(stack, require_name);
    if (new_require == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: change season: index %d: create require %s fail!",
            rank_g_svr_name(svr), index->m_id, require_name);
        return -1;
    }

    if (rank_g_svr_db_season_role_to_record_insert(svr, new_require, role_to_rank) != 0) {
        return -1;
    }
    
    return 0;
}

static int rank_g_svr_op_change_season_on_save_role_to_rank_result(
    rank_g_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    SVR_RANK_G_SEASON_SAVING * req, rank_g_svr_index_t index)
{
    rt_node_t node;

    if (req->rank_begin == req->rank_end) {
        assert(req->processing_count > 0);
        req->processing_count--;

        if (req->processing_count == 0) {
            rank_g_svr_op_change_season_save_done(svr, index);
        }
        return 0;
    }
    else {
        node = rt_find_by_rank(index->m_rank_tree, req->rank_begin);
        if (node == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: change season: index %d: first rank node %d not exist",
                rank_g_svr_name(svr), index->m_id, req->rank_begin);
            return -1;
        }

        if (rank_g_svr_op_change_season_save_record(svr, index, stack, node) != 0) return -1;

        req->rank_begin++;
        
        return 0;
    }
}

logic_context_t rank_g_svr_op_change_season_start(rank_g_svr_t svr, rank_g_svr_index_t index) {
    logic_context_t context;
    set_logic_rsp_carry_info_t carry_info;
    logic_data_t req_data;
    SVR_RANK_G_SEASON_SAVING * req;
                
    context = set_logic_rsp_manage_create_op_by_name(svr->m_rsp_manage, NULL, "internal_change_season", svr->m_em);
    if (context == NULL) {
        CPE_ERROR(svr->m_em, "%s: tick: index %d: create season change op fail!", rank_g_svr_name(svr), index->m_id);
        return NULL;
    }

    carry_info = set_logic_rsp_carry_info_find(context);
    if (carry_info) {
        set_logic_rsp_context_set_response(carry_info, 0);
    }
    
    req_data = logic_context_data_get_or_create(context, svr->m_meta_season_saving, 0);
    if (req_data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: tick: index %d: create season change op: alloc req data fail!",
            rank_g_svr_name(svr), index->m_id);
        logic_context_free(context);
        return NULL;
    }
    req = logic_data_data(req_data);

    req->index_id = index->m_id;
    req->processing_count = 0;
    req->rank_begin = 0;
    req->rank_end = rt_size(index->m_rank_tree);

    CPE_INFO(
        svr->m_em, "%s: index %d: create season change op: rank=[%d ~ %d)!",
        rank_g_svr_name(svr), index->m_id, req->rank_begin, req->rank_end);
    
    return context;
}

static int rank_g_svr_op_change_season_save_record(rank_g_svr_t svr, rank_g_svr_index_t index, logic_stack_node_t stack, rt_node_t node) {
    void * db_record;
    char require_name[64];
    logic_require_t require;
    SVR_RANK_G_SEASON_RECORD_COMMON * db_record_common;
    logic_data_t role_to_rank_data;
    SVR_RANK_G_SEASON_ROLE_TO_RANK * role_to_rank;
    uint64_t uin;
    
    db_record = rank_g_svr_build_db_record(svr, index, index->m_record_season, node);
    if (db_record == NULL) return -1;
    db_record_common = db_record;
    
    if (dr_entry_try_read_uint64(
            &uin,
            ((char*)db_record) + svr->m_season_record_data_start_pos + svr->m_uin_start_pos,
            svr->m_uin_entry, svr->m_em) != 0)
    {
        CPE_ERROR(svr->m_em, "%s: index %d: read uin fail", rank_g_svr_name(svr), index->m_id);
        return -1;
    }
    
    snprintf(require_name, sizeof(require_name), "record_%d", db_record_common->rank);
    require = logic_require_create(stack, require_name);
    if (require == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: tick: index %d: create require %s fail!",
            rank_g_svr_name(svr), index->m_id, require_name);
        return -1;
    }

    role_to_rank_data = logic_require_data_get_or_create(require, svr->m_meta_season_role_to_rank, 0);
    if (role_to_rank_data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: tick: index %d: require %s create role_to_rank data fail!",
            rank_g_svr_name(svr), index->m_id, require_name);
        logic_require_free(require);
        return -1;
    }
    role_to_rank = logic_data_data(role_to_rank_data);

    snprintf(
        role_to_rank->_id, sizeof(role_to_rank->_id), "%d-%d-" FMT_UINT64_T,
        db_record_common->index_id, db_record_common->season, uin);
    
    role_to_rank->index_id = db_record_common->index_id;
    role_to_rank->season = db_record_common->season;
    role_to_rank->rank = db_record_common->rank;

    if (rank_g_svr_db_season_record_insert(svr, require, db_record) != 0) return -1;

    return 0;
}

static SVR_RANK_G_SEASON_SAVING *
rank_g_svr_op_change_season_save_req(rank_g_svr_t svr, logic_context_t context) {
    logic_data_t req_data;
    req_data = logic_context_data_find(context, dr_meta_name(svr->m_meta_season_saving));
    assert(req_data);
    return logic_data_data(req_data);
}
    
static void rank_g_svr_op_change_season_save_done(rank_g_svr_t svr, rank_g_svr_index_t index) {
    rt_node_t node;
    uint32_t i;
    
    assert(svr->m_saving_error_time == 0);

    /*清理排行榜内所有记录的赛季信息，避免重启后进入排行榜 */
    for(node = rt_first(index->m_rank_tree); node; node = rt_next(index->m_rank_tree, node)) {
        void * record = aom_obj_get(svr->m_record_mgr, rt_node_record_id(node));

        assert(record);

        if (dr_entry_set_from_uint16(
                ((char *)record) + index->m_season_entry_start_pos, 0, index->m_season_entry, svr->m_em) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: index %d: season check update: record %s: set season to 0 fail!",
                rank_g_svr_name(svr), index->m_id, rank_g_svr_record_dump(svr, record));
        }
    }

    /*清理排行榜 */
    rt_tree_clear(index->m_rank_tree);
    for(i = 0; i < rt_capacity(index->m_rank_tree); ++i) {
        index->m_record_to_rank_pos[i] = INVALID_RANK_TREE_NODE_POS;
    }

    /*更新排行榜当前的赛季信息 */
    index->m_record_season = index->m_season_cur ? index->m_season_cur->m_season_id : 0;

    CPE_INFO(
        svr->m_em, "%s: index %d: season check update: op done, record season is %d!",
        rank_g_svr_name(svr), index->m_id, index->m_record_season);
}

static void rank_g_svr_op_change_season_save_error(rank_g_svr_t svr, rank_g_svr_index_t index, logic_stack_node_t stack) {
    if (svr->m_saving_error_time != 0) return;

    /*第一次发生错误 */
    svr->m_saving_error_time = rank_g_svr_cur_time(svr);
    logic_stack_node_cancel_all_requires(stack);
}
