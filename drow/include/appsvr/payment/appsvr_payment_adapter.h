#ifndef APPSVR_PAYMENT_ADAPTER_H
#define APPSVR_PAYMENT_ADAPTER_H
#include "protocol/appsvr/payment/appsvr_payment_pro.h"
#include "appsvr_payment_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*appsvr_payment_fini_fun_t)(appsvr_payment_adapter_t adapter);
typedef int (*appsvr_payment_start_fun_t)(appsvr_payment_adapter_t adapter, APPSVR_PAYMENT_BUY const * req);
typedef int (*appsvr_payment_query_products_fun_t)(appsvr_payment_adapter_t adapter);

appsvr_payment_adapter_t
appsvr_payment_adapter_create(
    appsvr_payment_module_t module, uint8_t service_type, const char * service_name,
    uint8_t support_restart, uint8_t support_sync,
    size_t capacity, appsvr_payment_start_fun_t start_fun,
    appsvr_payment_query_products_fun_t query_products);

void appsvr_payment_adapter_free(appsvr_payment_adapter_t adapter);

void * appsvr_payment_adapter_data(appsvr_payment_adapter_t adapter);

void appsvr_payment_adapter_set_fini(appsvr_payment_adapter_t adapter, appsvr_payment_fini_fun_t fini);

int appsvr_payment_adapter_notify_result(
    appsvr_payment_adapter_t adapter, APPSVR_PAYMENT_RESULT const * result);

int appsvr_payment_adapter_notify_product_sync_done(appsvr_payment_adapter_t adapter);
    
#ifdef __cplusplus
}
#endif

#endif
