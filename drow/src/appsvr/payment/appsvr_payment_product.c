#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "appsvr_payment_product_i.h"
#include "appsvr_payment_adapter_i.h"
#include "appsvr_payment_product_request_i.h"
#include "appsvr_payment_product_runing_i.h"

appsvr_payment_product_t
appsvr_payment_product_create(appsvr_payment_module_t module, appsvr_payment_adapter_t adapter, const char * product_id, const char * price) {
    appsvr_payment_product_t product;

    product = mem_alloc(module->m_alloc, sizeof(struct appsvr_payment_product));
    if (product == NULL) {
        CPE_ERROR(module->m_em, "appsvr_payment_product_create: alloc fail!");
        return NULL;
    }

    product->m_module = module;
    TAILQ_INSERT_TAIL(&module->m_products, product, m_next_for_module);
    
    product->m_adapter = adapter;
    TAILQ_INSERT_TAIL(&adapter->m_products, product, m_next_for_adapter);

    CPE_ERROR(module->m_em, "appsvr_payment_product_create: %s!",product_id);
    CPE_ERROR(module->m_em, "appsvr_payment_product_create: %s!",price);

    product->m_product_id = cpe_str_mem_dup_trim(module->m_alloc, product_id);
    product->m_price = cpe_str_mem_dup_trim(module->m_alloc, price);

    return product;
}

void appsvr_payment_product_free(appsvr_payment_product_t product) {
    appsvr_payment_module_t module = product->m_module;

    TAILQ_REMOVE(&product->m_module->m_products, product, m_next_for_module);
    TAILQ_REMOVE(&product->m_adapter->m_products, product, m_next_for_adapter);

    mem_free(module->m_alloc, product->m_product_id);
    mem_free(module->m_alloc, product->m_price);
    
    mem_free(module->m_alloc, product);
}

int appsvr_payment_module_query_products(
    appsvr_payment_module_t module,
    void * ctx, appsvr_payment_product_resonse_fun_t response_fun, void * arg, void (*arg_free_fun)(void * ctx))
{
    appsvr_payment_product_request_t request;
    appsvr_payment_adapter_t adapter;
    int rv = 0;
    
    request = appsvr_payment_product_request_create(module, ctx, response_fun, arg, arg_free_fun);
    if (request == NULL) {
        CPE_ERROR(module->m_em, "appsvr_payment_module_query_products: create rqeust fail!");
        return -1;
    }

    TAILQ_FOREACH(adapter, &module->m_adapters, m_next) {
        if (adapter->m_query_products == NULL) continue;

        if (adapter->m_product_sync_state == appsvr_payment_adapter_product_sync_init) {
            if (adapter->m_query_products(adapter) != 0) {
                CPE_ERROR(module->m_em, "appsvr_payment_module_query_products: adapter %d start query products fail!", adapter->m_service_type);
                rv = -1;
                break;
            }
            if (adapter->m_product_sync_state == appsvr_payment_adapter_product_sync_init) {
                adapter->m_product_sync_state = appsvr_payment_adapter_product_sync_runing;
            }
        }

        if (adapter->m_product_sync_state == appsvr_payment_adapter_product_sync_runing) {
            if (appsvr_payment_product_runing_create(request, adapter) == NULL) {
                CPE_ERROR(module->m_em, "appsvr_payment_module_query_products: adapter %d create runing fail!", adapter->m_service_type);
                rv = -1;
                break;
            }
        }
    }

    if (rv != 0) {
        appsvr_payment_product_request_free(request);
    }
    
    return rv;
}

const char* appsvr_payment_module_get_product(appsvr_payment_product_t t){
    return t ? t->m_product_id:NULL;
}

const char* appsvr_payment_module_get_price(appsvr_payment_product_t t){
    return t ? t->m_price:NULL;
}

appsvr_payment_product_t appsvr_payment_module_product_next(struct appsvr_payment_product_it * it) {
    appsvr_payment_product_t * data = (appsvr_payment_product_t *)(it->m_data);
    appsvr_payment_product_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next_for_module);
    return r;
}

void appsvr_payment_module_products(appsvr_payment_module_t module, appsvr_payment_product_it_t it) {
    *(appsvr_payment_product_t *)(it->m_data) = TAILQ_FIRST(&module->m_products);
    it->next = appsvr_payment_module_product_next;
}

appsvr_payment_product_t appsvr_payment_adapter_product_next(struct appsvr_payment_product_it * it) {
    appsvr_payment_product_t * data = (appsvr_payment_product_t *)(it->m_data);
    appsvr_payment_product_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next_for_adapter);
    return r;
}

void appsvr_payment_adapter_products(appsvr_payment_adapter_t adapter, appsvr_payment_product_it_t it) {
    *(appsvr_payment_product_t *)(it->m_data) = TAILQ_FIRST(&adapter->m_products);
    it->next = appsvr_payment_adapter_product_next;
}
