#include "rank_g_svr_season_info.h"

rank_g_svr_season_info_t
rank_g_svr_season_info_create(rank_g_svr_index_t index, uint16_t id, uint32_t begin_time, uint32_t end_time) {
    rank_g_svr_t svr = index->m_svr;
    rank_g_svr_season_info_t season_info;

    season_info = mem_alloc(svr->m_alloc, sizeof(struct rank_g_svr_season_info));
    if (season_info == NULL) {
        CPE_ERROR(svr->m_em, "rank_g_svr_season_info_create: alloc fail!");
        return NULL;
    }

    season_info->m_index = index;
    season_info->m_season_id = id;
    season_info->m_begin_time = begin_time;
    season_info->m_end_time = end_time;

    TAILQ_INSERT_TAIL(&index->m_season_infos, season_info, m_next);

    return season_info;
}

void rank_g_svr_season_info_free(rank_g_svr_season_info_t season_info) {
    rank_g_svr_t svr = season_info->m_index->m_svr;

    if (season_info->m_index->m_season_cur == season_info) {
        season_info->m_index->m_season_cur = NULL;
    }
    
    TAILQ_REMOVE(&season_info->m_index->m_season_infos, season_info, m_next);

    mem_free(svr->m_alloc, season_info);
}

rank_g_svr_season_info_t
rank_g_svr_season_info_find_by_id(rank_g_svr_index_t index, uint16_t id) {
    rank_g_svr_season_info_t season_info;

    TAILQ_FOREACH(season_info, &index->m_season_infos, m_next) {
        if (season_info->m_season_id == id) return season_info;
    }
    
    return NULL;
}
