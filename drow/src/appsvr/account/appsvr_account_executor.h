#ifndef APPSVR_ACCOUNT_EXECUTOR_H
#define APPSVR_ACCOUNT_EXECUTOR_H
#include "appsvr_account_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

int appsvr_account_executor_init(appsvr_account_module_t module);
void appsvr_account_executor_fini(appsvr_account_module_t module);

#ifdef __cplusplus
}
#endif

#endif
