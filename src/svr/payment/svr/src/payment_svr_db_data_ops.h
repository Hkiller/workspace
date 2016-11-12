#ifndef SVR_PAYMENT_SVR_DB_DATA_OPS_H
#define SVR_PAYMENT_SVR_DB_DATA_OPS_H
#include "payment_svr.h"


/*db ops*/
int payment_svr_db_send_query_money(
    payment_svr_t svr, BAG_INFO const * bag_info, logic_require_t require,
    uint64_t user_id);

int payment_svr_db_send_add_money(
    payment_svr_t svr, BAG_INFO const * bag_info, logic_require_t require,
    uint64_t user_id, SVR_PAYMENT_MONEY_GROUP const * diff);

int payment_svr_db_send_remove_money(
    payment_svr_t svr, BAG_INFO const * bag_info, logic_require_t require,
    uint64_t user_id, SVR_PAYMENT_MONEY_GROUP const * diff);

int payment_svr_db_send_init_money(
    payment_svr_t svr, BAG_INFO const * bag_info, logic_require_t require,
    uint64_t user_id, SVR_PAYMENT_MONEY_GROUP const * diff);

int payment_svr_db_build_balance(
    payment_svr_t svr, BAG_INFO const * bag_info, logic_require_t require,
    SVR_PAYMENT_MONEY_GROUP * balance);

#endif
