#ifndef SVR_GIFT_SVR_DB_OPS_H
#define SVR_GIFT_SVR_DB_OPS_H
#include "gift_svr.h"

#ifdef __cplusplus
extern "C" {
#endif

/*db ops*/
int gift_svr_db_send_use_query(gift_svr_t svr, logic_require_t require, const char * id);
int gift_svr_db_send_use_query_by_generate(gift_svr_t svr, logic_require_t require, uint32_t generate_id);

int gift_svr_db_send_use_insert(gift_svr_t svr, logic_require_t require, SVR_GIFT_USE_RECORD const * record);

int gift_svr_db_send_use_update_state(gift_svr_t svr, logic_require_t require, uint8_t from_state, SVR_GIFT_USE_RECORD const * record);

int gift_svr_db_send_use_remove_by_generate_id(gift_svr_t svr, logic_require_t require, uint32_t generate_id);

#ifdef __cplusplus
}
#endif
    
#endif
