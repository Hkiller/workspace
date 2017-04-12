#ifndef APPSVR_PAYMENT_PRODUCT_H
#define APPSVR_PAYMENT_PRODUCT_H
#include "protocol/appsvr/payment/appsvr_payment_pro.h"
#include "appsvr_payment_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_payment_product_it {
    appsvr_payment_product_t (*next)(struct appsvr_payment_product_it * it);
    char m_data[64];
};
    
appsvr_payment_product_t
appsvr_payment_product_create(
    appsvr_payment_module_t module,
    appsvr_payment_adapter_t adapter,
    const char * product_id,
    const char * price);

void appsvr_payment_product_free(appsvr_payment_product_t product);

void appsvr_payment_module_products(appsvr_payment_module_t module, appsvr_payment_product_it_t product_it);
void appsvr_payment_adapter_products(appsvr_payment_adapter_t adapter, appsvr_payment_product_it_t product_it);
    
appsvr_payment_product_t appsvr_payment_module_product_next(struct appsvr_payment_product_it * it);
appsvr_payment_product_t appsvr_payment_adapter_product_next(struct appsvr_payment_product_it * it);

const char* appsvr_payment_module_get_product(appsvr_payment_product_t t);
const char* appsvr_payment_module_get_price(appsvr_payment_product_t t);

#define appsvr_payment_product_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
