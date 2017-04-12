#ifndef APPSVR_NOTIFY_ADAPTER_H
#define APPSVR_NOTIFY_ADAPTER_H
#include "appsvr_notify_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*appsvr_notify_install_fun_t)(void * ctx, appsvr_notify_schedule_t schedule);
typedef int (*appsvr_notify_update_fun_t)(void * ctx, appsvr_notify_schedule_t schedule);
typedef void (*appsvr_notify_uninstall_fun_t)(void * ctx, appsvr_notify_schedule_t schedule);

appsvr_notify_adapter_t
appsvr_notify_adapter_create(
    appsvr_notify_module_t module, const char * name,
    void * ctx,
    appsvr_notify_install_fun_t install_fun,
    appsvr_notify_update_fun_t update_fun,
    appsvr_notify_uninstall_fun_t uninstall_fun,
    const char * tags);
    
void appsvr_notify_adapter_free(appsvr_notify_adapter_t adapter);
    
#ifdef __cplusplus
}
#endif

#endif
