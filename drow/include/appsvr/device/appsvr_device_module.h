#ifndef APPSVR_DEVICE_MODULE_H
#define APPSVR_DEVICE_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "appsvr_device_types.h"

#ifdef __cplusplus
extern "C" {
#endif

appsvr_device_module_t
appsvr_device_module_create(gd_app_context_t app, uint8_t debug, mem_allocrator_t alloc, const char * name, error_monitor_t em);

void appsvr_device_module_free(appsvr_device_module_t module);

appsvr_device_module_t appsvr_device_module_find(gd_app_context_t app, cpe_hash_string_t name);
appsvr_device_module_t appsvr_device_module_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t appsvr_device_module_app(appsvr_device_module_t module);
const char * appsvr_device_module_name(appsvr_device_module_t module);

#ifdef __cplusplus
}
#endif

#endif
