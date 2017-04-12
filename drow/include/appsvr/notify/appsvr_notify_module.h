#ifndef APPSVR_NOTIFY_MODULE_H
#define APPSVR_NOTIFY_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "appsvr_notify_types.h"

#ifdef __cplusplus
extern "C" {
#endif

appsvr_notify_module_t
appsvr_notify_module_create(
    gd_app_context_t app, uint8_t debug,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);

void appsvr_notify_module_free(appsvr_notify_module_t module);

appsvr_notify_module_t appsvr_notify_module_find(gd_app_context_t app, cpe_hash_string_t name);
appsvr_notify_module_t appsvr_notify_module_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t appsvr_notify_module_app(appsvr_notify_module_t module);
const char * appsvr_notify_module_name(appsvr_notify_module_t module);

void appsvr_notify_module_clear_in_tags(appsvr_notify_module_t module, const char * tags);

#ifdef __cplusplus
}
#endif

#endif
