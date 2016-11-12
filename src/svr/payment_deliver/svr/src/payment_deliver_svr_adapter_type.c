#include "payment_deliver_svr_adapter_type.h"

//PAYMENT_DECLARE_ADAPTER(iapppay);
PAYMENT_DECLARE_ADAPTER(qihoo);
PAYMENT_DECLARE_ADAPTER(damai);

struct payment_deliver_adapter_type g_adapter_types[] = {
    //PAYMENT_IMPL_ADAPTER(iapppay, PAYMENT_RECHARGE_SERVICE_IAPPPAY),
    PAYMENT_IMPL_ADAPTER(qihoo, PAYMENT_RECHARGE_SERVICE_QIHOO),
    PAYMENT_IMPL_ADAPTER(damai, PAYMENT_RECHARGE_SERVICE_DAMAI),
};

uint8_t g_adapter_type_count = CPE_ARRAY_SIZE(g_adapter_types);
