#include <assert.h>
#include "appsvr/payment/appsvr_payment_adapter.h"
#include "appsvr_unicompay_payment_adapter_i.h"

int appsvr_unicompay_payment_adapter_init(appsvr_unicompay_module_t module) {
    appsvr_unicompay_payment_adapter_t unicompay;

    assert(module->m_adapter == NULL);
    
    module->m_adapter = appsvr_payment_adapter_create(
        module->m_payment_module, appsvr_payment_service_unicompay_offline, "unicompay",
        0, 0,
        sizeof(struct appsvr_unicompay_payment_adapter), appsvr_unicompay_pay_start,NULL);
    if (module->m_adapter == NULL) {
        CPE_ERROR(module->m_em, "appsvr_payment_adapter_unicompay_create: create adapter fail!");
        return -1;
    }

    unicompay = (appsvr_unicompay_payment_adapter_t)appsvr_payment_adapter_data(module->m_adapter);

    unicompay->m_module = module;
    
    return 0;
}

void appsvr_unicompay_payment_adapter_fini(appsvr_unicompay_module_t module) {
    assert(module->m_adapter);

    appsvr_payment_adapter_free(module->m_adapter);
    
    module->m_adapter = NULL;
}
