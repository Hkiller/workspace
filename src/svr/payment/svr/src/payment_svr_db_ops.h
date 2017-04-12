#ifndef SVR_PAYMENT_SVR_DB_OPS_H
#define SVR_PAYMENT_SVR_DB_OPS_H
#include "payment_svr.h"

/*mongo_pkg utils function*/
int payment_svr_mongo_pkg_append_id(mongo_pkg_t db_pkg, uint64_t uid, uint16_t bag_id);
int payment_svr_mongo_pkg_append_required_moneies(mongo_pkg_t db_pkg, uint8_t mongo_type_count);

int payment_svr_db_validate_result(payment_svr_t svr, logic_require_t require);

int payment_svr_find_count_by_type(uint64_t * result, SVR_PAYMENT_MONEY_GROUP const * monies, uint8_t money_type);
uint64_t payment_svr_get_count_by_type(SVR_PAYMENT_MONEY_GROUP const * monies, uint8_t money_type, uint64_t dft);

#endif
