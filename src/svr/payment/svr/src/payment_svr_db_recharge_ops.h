#ifndef SVR_PAYMENT_SVR_DB_RECHARGE_OPS_H
#define SVR_PAYMENT_SVR_DB_RECHARGE_OPS_H
#include "payment_svr.h"

/*db ops*/
int payment_svr_db_recharge_send_insert(payment_svr_t svr, logic_require_t require, PAYMENT_RECHARGE_RECORD const * record);
int payment_svr_db_recharge_send_qurey_by_id(payment_svr_t svr, logic_require_t require, const char * recharge_id);
int payment_svr_db_recharge_send_update_state(payment_svr_t svr, logic_require_t require, PAYMENT_RECHARGE_RECORD const * record);

#endif
