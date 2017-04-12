#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "gd/timer/timer_manage.h"
#include "appsvr_haoxin_module_i.h"
#include "appsvr/payment/appsvr_payment_adapter.h"

static void appsvr_haoxin_module_clear_payment_cancel_timer(appsvr_haoxin_module_t module);

static int appsvr_haoxin_payment_pay_start(appsvr_payment_adapter_t adapter, APPSVR_PAYMENT_BUY const * req) {
    appsvr_haoxin_module_t module = *(appsvr_haoxin_module_t*)appsvr_payment_adapter_data(adapter);
    int rv;

    if (module->m_payment_runing) {
        CPE_ERROR(module->m_em, "appsvr_haoxin_payment_pay_start: call fail!");
        return -1;
    }

    rv = appsvr_haoxin_backend_pay_start(module, req);
    if (rv==0) {
        module->m_payment_runing = 1;
    }

    return rv;
}

void appsvr_haoxin_module_send_payment_result(appsvr_haoxin_module_t module, APPSVR_PAYMENT_RESULT const * payment_result) {
    if (!module->m_payment_runing) {
        CPE_ERROR(module->m_em, "appsvr_haoxin_module: send payment result: no payment runing!");
        return;
    }

    appsvr_haoxin_module_clear_payment_cancel_timer(module); /*取消保护的定时器 */
    module->m_payment_runing = 0;
    
    appsvr_payment_adapter_notify_result(module->m_payment_adapter, payment_result);  
}

static void appsvr_haoxin_module_on_runing_cancel(void * ctx, cpe_timer_id_t timer_id, void * arg) {
    appsvr_haoxin_module_t module = ctx;
    CPE_ERROR(module->m_em, "appsvr_haoxin_module_on_runing_cancel: enter!");

    assert(module->m_payment_cancel_timer == timer_id);
    module->m_payment_cancel_timer = GD_TIMER_ID_INVALID;
    
    if (module->m_payment_runing) {
        APPSVR_PAYMENT_RESULT payment_result;
        bzero(&payment_result, sizeof(payment_result));
        payment_result.result = appsvr_payment_canceled;
        payment_result.service_result = -1;
        module->m_payment_runing = 0;
        cpe_str_dup(payment_result.error_msg, sizeof(payment_result.error_msg), "");
        appsvr_payment_adapter_notify_result(module->m_payment_adapter, &payment_result);
        CPE_ERROR(module->m_em, "appsvr_haoxin_module_on_runing_cancel: enter success!");
    }
}

int appsvr_haoxin_module_start_payment_cancel_timer(appsvr_haoxin_module_t module) {
    gd_timer_mgr_t timer_mgr;

    timer_mgr = gd_timer_mgr_find_nc(module->m_app, NULL);
    if (timer_mgr == NULL) {
        CPE_ERROR(module->m_em, "appsvr_haoxin_module: start runint cancel timer: no timer manager!");
        return -1;
    }
    
    if (module->m_payment_cancel_timer != GD_TIMER_ID_INVALID) {
        gd_timer_mgr_unregist_timer_by_id(timer_mgr, module->m_payment_cancel_timer);
        module->m_payment_cancel_timer = GD_TIMER_ID_INVALID;
    }
    
    if (gd_timer_mgr_regist_timer(
            timer_mgr,
            &module->m_payment_cancel_timer,
            appsvr_haoxin_module_on_runing_cancel, module,
            NULL, NULL, 5000, 0, 1)
        != 0)
    {
        CPE_ERROR(module->m_em, "appsvr_haoxin_module: start runint cancel timer fail!");
        return -1;
    }

    CPE_ERROR(module->m_em, "appsvr_haoxin_module_start_payment_cancel_timer: enter success!");

    return 0;
}

static void appsvr_haoxin_module_clear_payment_cancel_timer(appsvr_haoxin_module_t module) {
    if (module->m_payment_cancel_timer != GD_TIMER_ID_INVALID) {
        gd_timer_mgr_t timer_mgr = gd_timer_mgr_find_nc(module->m_app, NULL);
        gd_timer_mgr_unregist_timer_by_id(timer_mgr, module->m_payment_cancel_timer);
        module->m_payment_cancel_timer = GD_TIMER_ID_INVALID;
    }
}

int appsvr_haoxin_payment_init(appsvr_haoxin_module_t module) {
	module->m_payment_adapter = 
		appsvr_payment_adapter_create(
		module->m_payment_module,
        appsvr_payment_service_haoxin_offline, "haoxin",
        0, 0,
		sizeof(appsvr_haoxin_module_t), appsvr_haoxin_payment_pay_start, NULL);
	if (module->m_payment_adapter == NULL) {
		CPE_ERROR(module->m_em, "appsvr_haoxin_login_init: create adapter fail!");
		return -1;
	}

	*(appsvr_haoxin_module_t*)appsvr_payment_adapter_data(module->m_payment_adapter) = module;

	return 0;
}

void appsvr_haoxin_payment_fini(appsvr_haoxin_module_t module) {
	if (module->m_payment_adapter) {
		appsvr_payment_adapter_free(module->m_payment_adapter);
		module->m_payment_adapter = NULL;
	}

    appsvr_haoxin_module_clear_payment_cancel_timer(module);
}
