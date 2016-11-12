#ifndef SVR_PAYMENT_SVR_DB_BILL_OPS_H
#define SVR_PAYMENT_SVR_DB_BILL_OPS_H
#include "payment_svr.h"

void payment_svr_db_add_bill(
    payment_svr_t svr, BAG_INFO const * bag_info, uint64_t user_id, 
    PAYMENT_BILL_DATA const * bill_data, SVR_PAYMENT_MONEY_GROUP const * balance);


#endif
