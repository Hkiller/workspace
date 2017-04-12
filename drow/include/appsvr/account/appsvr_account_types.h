#ifndef APPSVR_ACCOUNT_TYPES_H
#define APPSVR_ACCOUNT_TYPES_H
#include "cpe/utils/error.h"
#include "cpe/dr/dr_types.h"
#include "cpe/cfg/cfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct appsvr_account_module * appsvr_account_module_t;
typedef struct appsvr_account_adapter * appsvr_account_adapter_t;    

typedef appsvr_account_adapter_t
(*appsvr_account_adapter_creation_fun_t)(
    appsvr_account_module_t account_module, cfg_t cfg, mem_allocrator_t alloc, error_monitor_t em);
    
#ifdef __cplusplus
}
#endif

#endif
