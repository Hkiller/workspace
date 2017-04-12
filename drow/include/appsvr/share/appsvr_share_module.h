#ifndef APPSVR_SHARE_MODULE_H
#define APPSVR_SHARE_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "appsvr_share_types.h"

#ifdef __cplusplus
extern "C" {
#endif

appsvr_share_module_t
appsvr_share_module_create(
    gd_app_context_t app, uint8_t debug,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);

void appsvr_share_module_free(appsvr_share_module_t module);

appsvr_share_module_t appsvr_share_module_find(gd_app_context_t app, cpe_hash_string_t name);
appsvr_share_module_t appsvr_share_module_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t appsvr_share_module_app(appsvr_share_module_t module);
const char * appsvr_share_module_name(appsvr_share_module_t module);

#ifdef __cplusplus
}
#endif

#endif
