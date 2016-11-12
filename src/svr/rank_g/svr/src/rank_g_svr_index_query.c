#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/aom/aom_obj_hash.h"
#include "cpe/dr/dr_data.h"
#include "rank_g_svr_index.h"
#include "rank_g_svr_rank_tree.h"

static void rank_g_svr_index_query_visit_n(
    rank_g_svr_t svr, rank_g_svr_index_t index,
    rt_node_t rank_node, uint32_t require_count,
    rank_g_svr_record_visit_fun_t visit_fun, void * visit_ctx)
{
    while(require_count > 0 && rank_node) {
        void * record = aom_obj_get(svr->m_record_mgr, rt_node_record_id(rank_node));
        assert(record);
        visit_fun(visit_ctx, record, rank_node);

        --require_count;
        rank_node = rt_next(index->m_rank_tree, rank_node);
    }
}

static int rank_g_svr_index_query_by_pos(
    rank_g_svr_t svr, rank_g_svr_index_t index,
    SVR_RANK_G_QUERY_BY_POS const * req, uint32_t require_count,
    rank_g_svr_record_visit_fun_t visit_fun, void * visit_ctx)
{
    rt_node_t rank_node;

    assert(index);

    rank_node = rt_find_by_rank(index->m_rank_tree, req->start_pos);

    rank_g_svr_index_query_visit_n(svr, index, rank_node, require_count, visit_fun, visit_ctx);

    return 0;
}

static int rank_g_svr_index_query_by_id(
    rank_g_svr_t svr, rank_g_svr_index_t index,
    SVR_RANK_G_QUERY_BY_ID const * req, uint32_t require_count,
    rank_g_svr_record_visit_fun_t visit_fun, void * visit_ctx)
{
    uint64_t user_id = req->user_id;
    char buf[svr->m_record_size];
    rt_node_t rank_node;
    rt_node_t rank_node_last;
    uint32_t rank_value;
    void const * record;
    uint32_t record_index;

    assert(index);

    bzero(buf, sizeof(buf));
    if (dr_entry_set_from_uint64(buf + svr->m_uin_start_pos, user_id, svr->m_uin_entry, svr->m_em) != 0) {
        CPE_ERROR(svr->m_em, "%s: index %d: query by id: set role id fail", rank_g_svr_name(svr), index->m_id);
        return -1;
    }

    record = aom_obj_hash_table_find(svr->m_record_hash, buf);
    if (record == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: index %d: query by id: record of user " FMT_UINT64_T " not exist",
            rank_g_svr_name(svr), index->m_id, user_id);
        return SVR_RANK_G_ERROR_RECORD_NOT_EXIST;
    }
    record_index = aom_obj_index(svr->m_record_mgr, record);
    
    if (dr_entry_try_read_uint32(
            &rank_value,
            ((const char *)record) + index->m_rank_start_pos, index->m_rank_entry, svr->m_em)
        != 0)
    {
        CPE_ERROR(svr->m_em, "%s: index %d: query by user: read sort attr fail!", rank_g_svr_name(svr), index->m_id);
        return -1;
    }

    rank_node = rt_find_by_value_min(index->m_rank_tree, rank_value);
    rank_node_last = rt_find_by_value_max(index->m_rank_tree, rank_value);

    while(rank_node && rank_node != rank_node_last) {
        if (rank_node->m_record_id == record_index) break;
        rank_node = rt_next(index->m_rank_tree, rank_node);
    }

    if (rank_node == NULL || rank_node == rank_node_last) {
        CPE_ERROR(
            svr->m_em, "%s: index %d: query by id: record of user " FMT_UINT64_T " not exist, vlaue=%d, reocrd=%d",
            rank_g_svr_name(svr), index->m_id, user_id, rank_value, record_index);
        return -1;
    }

    if (req->start_pos_adj < 0) {
        uint32_t process_count = (uint32_t)(- req->start_pos_adj);
        rt_node_t p_node;
        
        for(p_node = rt_pre(index->m_rank_tree, rank_node);
            p_node && process_count > 0;
            rank_node = p_node, p_node = rt_pre(index->m_rank_tree, rank_node), --process_count)
        {
        }
    }
    else if (req->start_pos_adj > 0) {
        uint32_t process_count = (uint32_t)req->start_pos_adj;
        rt_node_t n_node;
        
        for(n_node = rt_next(index->m_rank_tree, rank_node);
            n_node && process_count > 0;
            rank_node = n_node, n_node = rt_next(index->m_rank_tree, rank_node), --process_count)
        {
        }
    }

    rank_g_svr_index_query_visit_n(svr, index, rank_node, require_count, visit_fun, visit_ctx);
    
    return 0;
}

int rank_g_svr_index_query(
    rank_g_svr_t svr, rank_g_svr_index_t index,
    SVR_RANK_G_QUERY const * query, uint32_t require_count,
    rank_g_svr_record_visit_fun_t visit_fun, void * visit_ctx)
{
    switch(query->query_type) {
    case SVR_RANK_G_QUERY_TYPE_BY_POS:
        return rank_g_svr_index_query_by_pos(svr, index, &query->query_data.by_pos, require_count, visit_fun, visit_ctx);
    case SVR_RANK_G_QUERY_TYPE_BY_ID:
        return rank_g_svr_index_query_by_id(svr, index, &query->query_data.by_id, require_count, visit_fun, visit_ctx);
    default:
        CPE_ERROR(svr->m_em, "%s: query: query type %d is unknoqn!", rank_g_svr_name(svr), query->query_type);
        return SVR_RANK_G_ERROR_QUERY_TYPE_UNKNOWN;
    }

    return 0;
}


