#ifndef SVR_PAYMENT_SVR_META_BAG_INFO_H
#define SVR_PAYMENT_SVR_META_BAG_INFO_H
#include "payment_svr.h"

BAG_INFO * payment_svr_meta_bag_info_find(payment_svr_t svr, uint16_t bag_id);
int payment_svr_meta_bag_info_load(payment_svr_t svr, cfg_t cfg);

uint8_t payment_svr_meta_bag_info_support_money_type(BAG_INFO const * bag_info, uint8_t money_type);

#endif
