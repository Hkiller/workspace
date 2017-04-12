#include "appsvr_payment_product_runing_i.h"
#include "appsvr_payment_adapter_i.h"

appsvr_payment_product_runing_t
appsvr_payment_product_runing_create(appsvr_payment_product_request_t request, appsvr_payment_adapter_t adapter) {
    appsvr_payment_module_t module = request->m_module;
    appsvr_payment_product_runing_t runing;

    runing = mem_alloc(module->m_alloc, sizeof(struct appsvr_payment_product_runing));
    if (runing == NULL) {
        CPE_ERROR(module->m_em, "appsvr_payment_product_runing_create: alloc fail!");
        return NULL;
    }

    runing->m_request = request;
    TAILQ_INSERT_TAIL(&request->m_runings, runing, m_next_for_request);
    
    runing->m_adapter = adapter;
    TAILQ_INSERT_TAIL(&adapter->m_runings, runing, m_next_for_adapter);
    
    return runing;
}

void appsvr_payment_product_runing_free(appsvr_payment_product_runing_t runing) {
    appsvr_payment_module_t module = runing->m_request->m_module;

    TAILQ_REMOVE(&runing->m_request->m_runings, runing, m_next_for_request);
    TAILQ_REMOVE(&runing->m_adapter->m_runings, runing, m_next_for_adapter);

    mem_free(module->m_alloc, runing);
}

