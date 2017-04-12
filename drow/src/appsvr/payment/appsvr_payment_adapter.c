#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin/app_env/plugin_app_env_request.h"
#include "appsvr_payment_adapter_i.h"
#include "appsvr_payment_product_i.h"
#include "appsvr_payment_product_runing_i.h"
#include "appsvr_payment_product_request_i.h"

appsvr_payment_adapter_t
appsvr_payment_adapter_create(
    appsvr_payment_module_t module, uint8_t service_type, const char * service_name,
    uint8_t support_restart, uint8_t support_sync,
    size_t capacity, appsvr_payment_start_fun_t start_fun, appsvr_payment_query_products_fun_t query_products)
{
    appsvr_payment_adapter_t adapter;

    adapter = mem_calloc(module->m_alloc, sizeof(struct appsvr_payment_adapter) + capacity);
    if (adapter == NULL) {
        CPE_ERROR(module->m_em, "payment: create adapter %s(%d): alloc fail!", service_name, service_type);
        return NULL;
    }

    adapter->m_module = module;
    adapter->m_service_type = service_type;
    cpe_str_dup(adapter->m_service_name, sizeof(adapter->m_service_name), service_name);
    adapter->m_support_restart = support_restart;
    adapter->m_support_sync = support_sync;
    adapter->m_product_sync_state = appsvr_payment_adapter_product_sync_init;
    adapter->m_fini = NULL;
    adapter->m_start_fun = start_fun;
    adapter->m_query_products = query_products;
    TAILQ_INIT(&adapter->m_products);
    TAILQ_INIT(&adapter->m_runings);

    module->m_adapter_count++;
    TAILQ_INSERT_TAIL(&module->m_adapters, adapter, m_next);
        
    return adapter;
}

void appsvr_payment_adapter_free(appsvr_payment_adapter_t adapter) {
    appsvr_payment_module_t module;

    module = adapter->m_module;

    if (module->m_runing_adapter == adapter) {
        module->m_runing_adapter = NULL;
        plugin_app_env_cancel_request_by_id(module->m_app_env, module->m_runing_id);
        module->m_runing_id = 0;
    }

    while(!TAILQ_EMPTY(&adapter->m_runings)) {
        appsvr_payment_product_runing_free(TAILQ_FIRST(&adapter->m_runings));
    }
    
    while(!TAILQ_EMPTY(&adapter->m_products)) {
        appsvr_payment_product_free(TAILQ_FIRST(&adapter->m_products));
    }

    if (adapter->m_fini) adapter->m_fini(adapter);
    
    module->m_adapter_count--;
    TAILQ_REMOVE(&module->m_adapters, adapter, m_next);

    mem_free(module->m_alloc, adapter);
}

void appsvr_payment_adapter_set_fini(appsvr_payment_adapter_t adapter, appsvr_payment_fini_fun_t fini) {
    adapter->m_fini = fini;
}

int appsvr_payment_adapter_notify_result(
    appsvr_payment_adapter_t adapter, APPSVR_PAYMENT_RESULT const * result)
{
    appsvr_payment_module_t module = adapter->m_module;
    int rv = 0;
    
    if (module->m_runing_adapter != adapter) {
        CPE_ERROR(module->m_em, "payment: notify result: runing adapter mismatch!");
        return -1;
    }

    if (plugin_app_env_notify_request_result(
            adapter->m_module->m_app_env, module->m_runing_id,
            0,
            module->m_meta_res_pay, result, sizeof(*result))
        != 0)
    {
        CPE_ERROR(module->m_em, "payment: notify result: notify result fail!");
        rv = -1;
    }

    module->m_runing_adapter = NULL;
    module->m_runing_id = 0;

    return rv;
}

int appsvr_payment_adapter_notify_product_sync_done(appsvr_payment_adapter_t adapter) {
    adapter->m_product_sync_state = appsvr_payment_adapter_product_sync_done;
    while(!TAILQ_EMPTY(&adapter->m_runings)) {
        appsvr_payment_product_runing_free(TAILQ_FIRST(&adapter->m_runings));
    }
    return 0;
}

void * appsvr_payment_adapter_data(appsvr_payment_adapter_t adapter) {
    return adapter + 1;
}

