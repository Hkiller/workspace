#include <assert.h>
#include "appsvr_googlepay_backend.hpp"

int appsvr_googlepay_backend_init(appsvr_googlepay_module_t module) {
    return 0;
}

void appsvr_googlepay_backend_fini(appsvr_googlepay_module_t module) {
}

int appsvr_googlepay_backend_pay_start(appsvr_googlepay_module_t module, APPSVR_PAYMENT_BUY const * req) {
    appsvr_googlepay_module_start_payment_success_timer(module);
    return 0;
}

int appsvr_googlepay_backend_do_sync_products(appsvr_googlepay_module_t module)
{
	return 0;
}