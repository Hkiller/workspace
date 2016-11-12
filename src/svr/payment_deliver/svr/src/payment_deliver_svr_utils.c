#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/error.h"
#include "payment_deliver_svr_utils.h"

int payment_deliver_svr_parse_product_id(
    uint8_t * device_category, uint16_t * set_id, uint16_t * service, const char * order_id, error_monitor_t em)
{
    char buf[32];
    const char * sep;
    int a1, a2, a3;
    
    sep = strchr(order_id, '-');
    if (sep == NULL) {
        CPE_ERROR(em, "payment_deliver_svr_parse_product_id: order %s format error(no sep)", order_id);
        return -1;
    }

    if ((sep - order_id) != (1 + 5 + 3)) {
        CPE_ERROR(em, "payment_deliver_svr_parse_product_id: order %s format error(prefix len %d error)", order_id, (int)(sep - order_id));
        return -1;
    }

    cpe_str_dup_range(buf, sizeof(buf), order_id, sep);

    if (sscanf(buf, "%1d%05d%03d", &a1, &a2, &a3) != 3) {
        CPE_ERROR(em, "payment_deliver_svr_parse_product_id: order %s format error(scan error)", order_id);
        return -1;
    }

    *device_category = (uint8_t)a1;
    *set_id = (uint16_t)a2;
    *service = (uint16_t)a3;

    return 0;
}

