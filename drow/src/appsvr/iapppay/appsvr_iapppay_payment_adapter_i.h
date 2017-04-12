#ifndef APPSVR_IAPPPAY_PAYMENT_ADAPTER_H
#define APPSVR_IAPPPAY_PAYMENT_ADAPTER_H
#include "appsvr_iapppay_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_iapppay_payment_adapter {
    appsvr_iapppay_module_t m_module;
};

appsvr_payment_adapter_t appsvr_iapppay_payment_adapter_create(appsvr_iapppay_module_t module);
    
int appsvr_iapppay_pay_start(appsvr_payment_adapter_t adapter, APPSVR_PAYMENT_BUY const * req);

#ifdef __cplusplus
}
#endif

#endif
