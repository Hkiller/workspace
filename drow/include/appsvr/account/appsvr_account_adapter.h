#ifndef APPSVR_ACCOUNT_ADAPTER_H
#define APPSVR_ACCOUNT_ADAPTER_H
#include "protocol/appsvr/account/appsvr_account_pro.h"
#include "appsvr_account_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*appsvr_account_fini_fun_t)(appsvr_account_adapter_t adapter);
typedef int (*appsvr_account_login_start_fun_t)(appsvr_account_adapter_t adapter, APPSVR_ACCOUNT_LOGIN const * req);
typedef int (*appsvr_account_relogin_start_fun_t)(appsvr_account_adapter_t adapter, APPSVR_ACCOUNT_RELOGIN const * req);

appsvr_account_adapter_t
appsvr_account_adapter_create(
    appsvr_account_module_t module, uint8_t service_type, const char * service_name,
    size_t capacity, 
    appsvr_account_login_start_fun_t login_start_fun,
    appsvr_account_relogin_start_fun_t relogin_start_fun);

void appsvr_account_adapter_free(appsvr_account_adapter_t adapter);

void * appsvr_account_adapter_data(appsvr_account_adapter_t adapter);

void appsvr_account_adapter_set_fini(appsvr_account_adapter_t adapter, appsvr_account_fini_fun_t fini);

int appsvr_account_adapter_notify_login_result(
    appsvr_account_adapter_t adapter, APPSVR_ACCOUNT_LOGIN_RESULT const * result);

#ifdef __cplusplus
}
#endif

#endif
