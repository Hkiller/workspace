#ifndef SVR_RANK_G_SVR_DB_RECORD_H
#define SVR_RANK_G_SVR_DB_RECORD_H
#include "rank_g_svr.h"

#ifdef __cplusplus
extern "C" {
#endif

/*db ops*/
void * rank_g_svr_build_db_record(rank_g_svr_t svr, rank_g_svr_index_t index, uint16_t season, rt_node_t node);
    
#ifdef __cplusplus
}
#endif
    
#endif
