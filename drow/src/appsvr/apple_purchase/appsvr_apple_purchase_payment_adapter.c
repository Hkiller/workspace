#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "gd/timer/timer_manage.h"
#include "appsvr_apple_purchase_module_i.h"
#include "appsvr/payment/appsvr_payment_adapter.h"

static void appsvr_apple_purchase_module_clear_payment_cancel_timer(appsvr_apple_purchase_module_t module);

static int appsvr_apple_purchase_payment_pay_start(appsvr_payment_adapter_t adapter, APPSVR_PAYMENT_BUY const * req) {
    appsvr_apple_purchase_module_t module = *(appsvr_apple_purchase_module_t*)appsvr_payment_adapter_data(adapter);

    return  appsvr_apple_purchase_backend_pay_start(module, req);
}

static int appsvr_apple_purchase_payment_sync_products(appsvr_payment_adapter_t adapter) {
    appsvr_apple_purchase_module_t module = *(appsvr_apple_purchase_module_t*)appsvr_payment_adapter_data(adapter);

    return appsvr_apple_purchase_backend_do_sync_products(module);
}

static void appsvr_apple_purchase_module_on_runing_success(void * ctx, cpe_timer_id_t timer_id, void * arg) {
    appsvr_apple_purchase_module_t module = (appsvr_apple_purchase_module_t)ctx;
    APPSVR_PAYMENT_RESULT payment_result;

    CPE_ERROR(module->m_em, "appsvr_apple_purchase_module_on_runing_cancel: enter!");

    assert(module->m_payment_success_timer == timer_id);
    module->m_payment_success_timer = GD_TIMER_ID_INVALID;

    bzero(&payment_result, sizeof(payment_result));
    payment_result.result = appsvr_payment_success;
    payment_result.service_result = 0;

    cpe_str_dup(payment_result.error_msg, sizeof(payment_result.error_msg), "");
    appsvr_payment_adapter_notify_result(module->m_payment_adapter, &payment_result);
    CPE_ERROR(module->m_em, "appsvr_apple_purchase_module_on_runing_success: enter success!");
}

int appsvr_apple_purchase_module_start_payment_success_timer(appsvr_apple_purchase_module_t module) {
    gd_timer_mgr_t timer_mgr;

    timer_mgr = gd_timer_mgr_find_nc(module->m_app, NULL);
    if (timer_mgr == NULL) {
        CPE_ERROR(module->m_em, "appsvr_apple_purchase_module: start runint cancel timer: no timer manager!");
        return -1;
    }

    if (module->m_payment_success_timer != GD_TIMER_ID_INVALID) {
        gd_timer_mgr_unregist_timer_by_id(timer_mgr, module->m_payment_success_timer);
        module->m_payment_success_timer = GD_TIMER_ID_INVALID;
    }

    if (gd_timer_mgr_regist_timer(
        timer_mgr,
        &module->m_payment_success_timer,
        appsvr_apple_purchase_module_on_runing_success, module,
        NULL, NULL, 5000, 0, 1)
        != 0)
    {
        CPE_ERROR(module->m_em, "appsvr_apple_purchase_module: start runint success timer fail!");
        return -1;
    }

    CPE_ERROR(module->m_em, "appsvr_apple_purchase_module_start_payment_success_timer: enter success!");

    return 0;
}

int appsvr_apple_purchase_payment_init(appsvr_apple_purchase_module_t module) {
	module->m_payment_adapter = 
		appsvr_payment_adapter_create(
		module->m_payment_module,
        appsvr_payment_service_applepurchasepay_offline, "apple_purchase",
        0, 0,
		sizeof(appsvr_apple_purchase_module_t),
        appsvr_apple_purchase_payment_pay_start,
        appsvr_apple_purchase_payment_sync_products);
	if (module->m_payment_adapter == NULL) {
		CPE_ERROR(module->m_em, "appsvr_apple_purchase_login_init: create adapter fail!");
		return -1;
	}

	*(appsvr_apple_purchase_module_t*)appsvr_payment_adapter_data(module->m_payment_adapter) = module;

	return 0;
}

void appsvr_apple_purchase_payment_fini(appsvr_apple_purchase_module_t module) {
	if (module->m_payment_adapter) {
		appsvr_payment_adapter_free(module->m_payment_adapter);
		module->m_payment_adapter = NULL;
	}
}
