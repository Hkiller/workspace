#ifndef SVR_RANK_G_SVR_INDEX_H
#define SVR_RANK_G_SVR_INDEX_H
#include "rank_g_svr.h"

#ifdef __cplusplus
extern "C" {
#endif

struct rank_g_svr_index {
    rank_g_svr_t m_svr;
    TAILQ_ENTRY(rank_g_svr_index) m_next;
    uint16_t m_id;

    /*season info*/
    uint8_t m_season_use;
    uint16_t m_season_keep_count;
    LPDRMETAENTRY m_season_entry;
    uint16_t m_season_entry_start_pos;
    rank_g_svr_season_info_t m_season_cur;
    rank_g_svr_season_info_list_t m_season_infos;

    /*rank info*/
    LPDRMETAENTRY m_rank_entry;
    uint16_t m_rank_start_pos;

    /*data */
    uint16_t m_record_season;
    uint32_t * m_record_to_rank_pos;
    void * m_rank_tree_buff;
    rt_t m_rank_tree;
};

/*index*/
rank_g_svr_index_t
rank_g_svr_index_create(rank_g_svr_t svr, uint16_t id, const char * entry_path);
void rank_g_svr_index_free(rank_g_svr_index_t index);

int rank_g_svr_index_set_season_info(rank_g_svr_index_t index, uint16_t keep_count, const char * season_attr);

rank_g_svr_index_t rank_g_svr_index_find(rank_g_svr_t svr, uint16_t id);

int rank_g_svr_index_init_record(rank_g_svr_index_t index, uint32_t record_count);

int rank_g_svr_index_season_check_update(rank_g_svr_index_t index, void const * record);
    
int rank_g_svr_index_update(rank_g_svr_index_t index, void const * record, uint32_t record_id);
void rank_g_svr_index_remove(rank_g_svr_index_t index, uint32_t record_id);

typedef int (*rank_g_svr_record_visit_fun_t)(void * ctx, void const * record, rt_node_t node);

int rank_g_svr_index_query(
    rank_g_svr_t svr, rank_g_svr_index_t index,
    SVR_RANK_G_QUERY const * query, uint32_t require_count,
    rank_g_svr_record_visit_fun_t visit_fun, void * visit_ctx);

#ifdef __cplusplus
}
#endif

#endif
