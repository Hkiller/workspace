#ifndef SVR_GIFT_SVR_DB_OPS_GENERATE_H
#define SVR_GIFT_SVR_DB_OPS_GENERATE_H
#include "gift_svr.h"

#ifdef __cplusplus
extern "C" {
#endif

int gift_svr_db_send_generate_record_query(gift_svr_t svr, logic_require_t require);
int gift_svr_db_send_generate_insert(gift_svr_t svr, logic_require_t require, void const * record_data);
int gift_svr_db_send_generate_update_duration(gift_svr_t svr, logic_require_t require, uint32_t generate_id, uint32_t begin_time, uint32_t expire_time);
int gift_svr_db_send_generate_remove(gift_svr_t svr, logic_require_t require, uint32_t generate_id);

#ifdef __cplusplus
}
#endif
    
#endif
