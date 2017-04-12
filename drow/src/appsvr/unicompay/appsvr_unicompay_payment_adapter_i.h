#ifndef APPSVR_IAPPPAY_PAYMENT_ADAPTER_H
#define APPSVR_IAPPPAY_PAYMENT_ADAPTER_H
#include "appsvr_unicompay_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_unicompay_payment_adapter {
    appsvr_unicompay_module_t m_module;
};

appsvr_payment_adapter_t appsvr_unicompay_payment_adapter_create(appsvr_unicompay_module_t module);
    
int appsvr_unicompay_pay_start(appsvr_payment_adapter_t adapter, APPSVR_PAYMENT_BUY const * req);

#ifdef __cplusplus
}
#endif

#endif
