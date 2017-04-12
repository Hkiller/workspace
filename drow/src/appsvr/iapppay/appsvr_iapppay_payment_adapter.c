#include <assert.h>
#include "appsvr/payment/appsvr_payment_adapter.h"
#include "appsvr_iapppay_payment_adapter_i.h"

int appsvr_iapppay_payment_adapter_init(appsvr_iapppay_module_t module) {
    appsvr_iapppay_payment_adapter_t iapppay;

    assert(module->m_adapter == NULL);
    
    module->m_adapter = appsvr_payment_adapter_create(
        module->m_payment_module, appsvr_payment_service_iapppay, "iapppay",
        1, 1,
        sizeof(struct appsvr_iapppay_payment_adapter), appsvr_iapppay_pay_start, NULL);
    if (module->m_adapter == NULL) {
        CPE_ERROR(module->m_em, "appsvr_payment_adapter_iapppay_create: create adapter fail!");
        return -1;
    }

    iapppay = (appsvr_iapppay_payment_adapter_t)appsvr_payment_adapter_data(module->m_adapter);

    iapppay->m_module = module;
    
    return 0;
}

void appsvr_iapppay_payment_adapter_fini(appsvr_iapppay_module_t module) {
    assert(module->m_adapter);

    appsvr_payment_adapter_free(module->m_adapter);
    
    module->m_adapter = NULL;
}
