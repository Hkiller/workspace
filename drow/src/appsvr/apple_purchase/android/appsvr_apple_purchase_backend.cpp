#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_calc.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "appsvr_apple_purchase_backend.hpp"
#include "appsvr/payment/appsvr_payment_adapter.h"

int appsvr_apple_purchase_backend_pay_start(appsvr_apple_purchase_module_t module, APPSVR_PAYMENT_BUY const * req) {
    return 0;
}

int appsvr_apple_purchase_backend_init(appsvr_apple_purchase_module_t module) {
    return 0;
}

void appsvr_apple_purchase_backend_fini(appsvr_apple_purchase_module_t module) {
}
