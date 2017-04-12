#ifndef DROW_PLUGIN_PACK_MODULE_H
#define DROW_PLUGIN_PACK_MODULE_H
#include "gd/app/app_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "cpe/dr/dr_types.h"
#include "plugin_pack_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_pack_module_t
plugin_pack_module_create(
    gd_app_context_t app, ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr, ui_ed_mgr_t ed_mgr,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);

void plugin_pack_module_free(plugin_pack_module_t mgr);

plugin_pack_module_t plugin_pack_module_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_pack_module_t plugin_pack_module_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t plugin_pack_module_app(plugin_pack_module_t mgr);
const char * plugin_pack_module_name(plugin_pack_module_t mgr);

#ifdef __cplusplus
}
#endif

#endif 

