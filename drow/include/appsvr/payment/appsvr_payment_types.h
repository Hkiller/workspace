#ifndef APPSVR_PAYMENT_TYPES_H
#define APPSVR_PAYMENT_TYPES_H
#include "cpe/utils/error.h"
#include "cpe/dr/dr_types.h"
#include "cpe/cfg/cfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct appsvr_payment_module * appsvr_payment_module_t;
typedef struct appsvr_payment_adapter * appsvr_payment_adapter_t;    
typedef struct appsvr_payment_product * appsvr_payment_product_t;    
typedef struct appsvr_payment_product_it * appsvr_payment_product_it_t;

typedef appsvr_payment_adapter_t
(*appsvr_payment_adapter_creation_fun_t)(
    appsvr_payment_module_t payment_module, cfg_t cfg, mem_allocrator_t alloc, error_monitor_t em);

typedef int (*appsvr_payment_product_resonse_fun_t)(void * ctx, appsvr_payment_module_t payment_module, void * args);
    
#ifdef __cplusplus
}
#endif

#endif
