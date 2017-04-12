#ifndef PLUGIN_MASK_MANIP_MANIP_H
#define PLUGIN_MASK_MANIP_MANIP_H
#include "cpe/vfs/vfs_types.h"
#include "gd/app/app_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "plugin_mask_manip_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_mask_manip_t
plugin_mask_manip_create(
    gd_app_context_t app,
    plugin_package_manip_t package_manip,
    plugin_mask_module_t mask_module,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);

void plugin_mask_manip_free(plugin_mask_manip_t manip);

plugin_mask_manip_t plugin_mask_manip_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_mask_manip_t plugin_mask_manip_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t plugin_mask_manip_app(plugin_mask_manip_t manip);
const char * plugin_mask_manip_name(plugin_mask_manip_t manip);

#ifdef __cplusplus
}
#endif

#endif
