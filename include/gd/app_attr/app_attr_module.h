#ifndef GD_APP_ATTR_MODULE_H
#define GD_APP_ATTR_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "app_attr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

app_attr_module_t
app_attr_module_create(
    gd_app_context_t app, uint8_t debug,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);

void app_attr_module_free(app_attr_module_t module);

app_attr_module_t app_attr_module_find(gd_app_context_t app, cpe_hash_string_t name);
app_attr_module_t app_attr_module_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t app_attr_module_app(app_attr_module_t module);
const char * app_attr_module_name(app_attr_module_t module);

#ifdef __cplusplus
}
#endif

#endif
