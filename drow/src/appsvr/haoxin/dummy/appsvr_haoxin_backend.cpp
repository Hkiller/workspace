#include <assert.h>
#include "appsvr_haoxin_backend.hpp"

int appsvr_haoxin_backend_init(appsvr_haoxin_module_t module) {
    appsvr_haoxin_notify_support_more_game(module, (uint8_t)0);
    appsvr_haoxin_notify_support_exit_game(module, (uint8_t)0);
    appsvr_haoxin_notify_payscreen(module,"A");
    return 0;
}

void appsvr_haoxin_backend_fini(appsvr_haoxin_module_t module) {
}

void appsvr_haoxin_on_suspend(appsvr_haoxin_module_t module) {
}

void appsvr_haoxin_on_resume(appsvr_haoxin_module_t module) {
}

int appsvr_haoxin_backend_pay_start(appsvr_haoxin_module_t module, APPSVR_PAYMENT_BUY const * req) {
    return 0;
}
