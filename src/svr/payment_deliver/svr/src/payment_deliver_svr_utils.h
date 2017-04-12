#ifndef SVR_PAYMENT_DELIVER_SVR_UTILS_H
#define SVR_PAYMENT_DELIVER_SVR_UTILS_H
#include "payment_deliver_svr.h"

int payment_deliver_svr_parse_product_id(
    uint8_t * device_category, uint16_t * set_id, uint16_t * service, const char * order_id, error_monitor_t em);

#endif
