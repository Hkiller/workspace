#include <assert.h>
#include "gd/timer/timer_manage.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "appsvr_chuangku_backend.hpp"
#include "appsvr/payment/appsvr_payment_adapter.h"

int appsvr_chuangku_backend_init(appsvr_chuangku_module_t module) {
   // appsvr_chuangku_notify_support_more_game(module, (uint8_t)0);
    //appsvr_chuangku_notify_support_exit_game(module, (uint8_t)0);
    return 0;
}

void appsvr_chuangku_backend_fini(appsvr_chuangku_module_t module) {
}

int appsvr_chuangku_sync_addition_attr(appsvr_chuangku_module_t module) {
    return 0;
}

int appsvr_chuangku_sync_attr(appsvr_chuangku_module_t module) {
    return 0;
}

void appsvr_chuangku_on_suspend(appsvr_chuangku_module_t module) {
}

void appsvr_chuangku_on_resume(appsvr_chuangku_module_t module) {
}

int appsvr_chuangku_backend_pay_start(appsvr_chuangku_module_t module, APPSVR_PAYMENT_BUY const * req) {
    appsvr_chuangku_module_start_payment_success_timer(module);
    return 0;
}
