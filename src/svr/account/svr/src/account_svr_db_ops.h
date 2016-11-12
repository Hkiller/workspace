#ifndef SVR_ACCOUNT_SVR_DB_OPS_H
#define SVR_ACCOUNT_SVR_DB_OPS_H
#include "cpe/utils/hash_string.h"
#include "account_svr_module.h"

/*db ops*/
int account_svr_db_send_query_by_logic_id(
    account_svr_t svr, logic_require_t require,
    SVR_ACCOUNT_LOGIC_ID const * logic_id, LPDRMETA result_meta);
int account_svr_db_send_insert(
    account_svr_t svr, logic_require_t require, 
    uint64_t account_id, SVR_ACCOUNT_LOGIC_ID const * logic_id, uint8_t account_state);
int account_svr_db_send_bind(
    account_svr_t svr, logic_require_t require, 
    uint64_t account_id, SVR_ACCOUNT_LOGIC_ID const * logic_id);
int account_svr_db_send_unbind(
    account_svr_t svr, logic_require_t require, 
    uint64_t account_id, uint16_t account_type);

#endif
