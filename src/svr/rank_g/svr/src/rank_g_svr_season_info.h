#ifndef SVR_RANK_G_SVR_SEASON_INFO_H
#define SVR_RANK_G_SVR_SEASON_INFO_H
#include "rank_g_svr_index.h"

#ifdef __cplusplus
extern "C" {
#endif

struct rank_g_svr_season_info {
    rank_g_svr_index_t m_index;
    TAILQ_ENTRY(rank_g_svr_season_info) m_next;
    uint16_t m_season_id;
    uint32_t m_begin_time;
    uint32_t m_end_time;
};

/*season_info*/
rank_g_svr_season_info_t
rank_g_svr_season_info_create(rank_g_svr_index_t index, uint16_t id, uint32_t begin_time, uint32_t end_time);
void rank_g_svr_season_info_free(rank_g_svr_season_info_t season_info);

rank_g_svr_season_info_t rank_g_svr_season_info_find_by_id(rank_g_svr_index_t index, uint16_t id);

#ifdef __cplusplus
}
#endif

#endif
