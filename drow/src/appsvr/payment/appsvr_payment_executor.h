#ifndef APPSVR_PAYMENT_EXECUTOR_H
#define APPSVR_PAYMENT_EXECUTOR_H
#include "appsvr_payment_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

int appsvr_payment_executor_init(appsvr_payment_module_t module);
void appsvr_payment_executor_fini(appsvr_payment_module_t module);

#ifdef __cplusplus
}
#endif

#endif
