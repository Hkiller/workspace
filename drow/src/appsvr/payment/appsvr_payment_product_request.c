#include "appsvr_payment_product_request_i.h"
#include "appsvr_payment_product_runing_i.h"

appsvr_payment_product_request_t 
appsvr_payment_product_request_create(
    appsvr_payment_module_t module,
    void * ctx, appsvr_payment_product_resonse_fun_t response_fun, void * arg, void (*arg_free_fun)(void * ctx))
{
    appsvr_payment_product_request_t request;

    request = mem_alloc(module->m_alloc, sizeof(struct appsvr_payment_product_request));
    if (request == NULL) {
        CPE_ERROR(module->m_em, "appsvr_payment_product_request_create: alloc fail!");
        return NULL;
    }

    request->m_module = module;
    request->m_ctx = ctx;
    request->m_response_fun = response_fun;
    request->m_arg = arg;
    request->m_arg_free_fun = arg_free_fun;
    TAILQ_INIT(&request->m_runings);
    
    TAILQ_INSERT_TAIL(&module->m_product_requests, request, m_next_for_module);
    
    return request;
}

void appsvr_payment_product_request_free(appsvr_payment_product_request_t request) {
    appsvr_payment_module_t module = request->m_module;

    while(!TAILQ_EMPTY(&request->m_runings)) {
        appsvr_payment_product_runing_free(TAILQ_FIRST(&request->m_runings));
    }

    if (request->m_arg_free_fun) {
        request->m_arg_free_fun(request->m_arg);
    }
    
    TAILQ_REMOVE(&module->m_product_requests, request, m_next_for_module);
    
    mem_free(module->m_alloc, request);
}

