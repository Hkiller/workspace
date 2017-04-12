#include <assert.h>
#include "appsvr_apple_purchase_backend.hpp"

int appsvr_apple_purchase_backend_init(appsvr_apple_purchase_module_t module) {
    return 0;
}

void appsvr_apple_purchase_backend_fini(appsvr_apple_purchase_module_t module) {
}

int appsvr_apple_purchase_backend_pay_start(appsvr_apple_purchase_module_t module, APPSVR_PAYMENT_BUY const * req) {
    appsvr_apple_purchase_module_start_payment_success_timer(module);
    return 0;
}

int appsvr_apple_purchase_backend_do_sync_products(appsvr_apple_purchase_module_t module) {
    return 0;
}

