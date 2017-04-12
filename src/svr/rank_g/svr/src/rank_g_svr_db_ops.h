#ifndef SVR_RANK_G_SVR_DB_OPS_H
#define SVR_RANK_G_SVR_DB_OPS_H
#include "rank_g_svr.h"

#ifdef __cplusplus
extern "C" {
#endif

/*db ops*/
int rank_g_svr_db_send_query_by_rank(rank_g_svr_t svr, logic_require_t require, uint16_t season, uint32_t rank, uint32_t require_count);
int rank_g_svr_db_send_query_by_role_id(rank_g_svr_t svr, logic_require_t require, uint16_t season, uint32_t rank, uint32_t require_count);
    
int rank_g_svr_db_season_record_insert(rank_g_svr_t svr, logic_require_t require, void const * db_record);
int rank_g_svr_db_season_role_to_record_insert(rank_g_svr_t svr, logic_require_t require, SVR_RANK_G_SEASON_ROLE_TO_RANK const * role_to_rank);

#ifdef __cplusplus
}
#endif
    
#endif
