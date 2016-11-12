#include <assert.h>
#include "cpe/aom/aom_obj_mgr.h"
#include "rank_g_svr_db_record.h"
#include "rank_g_svr_index.h"
#include "rank_g_svr_rank_tree.h"

void * rank_g_svr_build_db_record(rank_g_svr_t svr, rank_g_svr_index_t index, uint16_t season, rt_node_t node) {
    void * db_record;
    SVR_RANK_G_SEASON_RECORD_COMMON * db_record_common;
    void const * mem_record = aom_obj_get(svr->m_record_mgr, rt_node_record_id(node));
    assert(mem_record);

    db_record = rank_g_svr_record_buf(svr, svr->m_season_record_size);
    if (db_record == NULL) {
        CPE_ERROR(svr->m_em, "rank_g_svr_build_db_record: alloc record fail, size=%d", svr->m_season_record_size);
        return NULL;
    }
    
    db_record_common = db_record;
    db_record_common->index_id = index->m_id;
    db_record_common->season = season;
    db_record_common->rank = rt_node_rank(index->m_rank_tree, node);
    db_record_common->_id = (((uint64_t)index->m_id)) << 48 | (((uint64_t)db_record_common->season) << 32) | (uint64_t)db_record_common->rank;

    memcpy(
        ((char*)db_record) + svr->m_season_record_data_start_pos,
        mem_record, svr->m_record_size);

    return db_record;
}
