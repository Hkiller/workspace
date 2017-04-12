#ifndef APPSVR_PAYMENT_MODULE_H
#define APPSVR_PAYMENT_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "appsvr_payment_types.h"

#ifdef __cplusplus
extern "C" {
#endif

appsvr_payment_module_t
appsvr_payment_module_create(gd_app_context_t app, uint8_t debug, mem_allocrator_t alloc, const char * name, error_monitor_t em);

void appsvr_payment_module_free(appsvr_payment_module_t module);

appsvr_payment_module_t appsvr_payment_module_find(gd_app_context_t app, cpe_hash_string_t name);
appsvr_payment_module_t appsvr_payment_module_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t appsvr_payment_module_app(appsvr_payment_module_t module);
const char * appsvr_payment_module_name(appsvr_payment_module_t module);

int appsvr_payment_module_query_products(
    appsvr_payment_module_t module,
    void * ctx, appsvr_payment_product_resonse_fun_t response_fun, void * arg, void (*arg_free_fun)(void * ctx));
    
#ifdef __cplusplus
}
#endif

#endif
