#ifndef DROW_PLUGIN_BASICANIM_MODULE_H
#define DROW_PLUGIN_BASICANIM_MODULE_H
#include "gd/app/app_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "cpe/dr/dr_types.h"
#include "plugin_basicanim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_basicanim_module_t
plugin_basicanim_module_create(
    gd_app_context_t app, ui_data_mgr_t data_mgr, ui_runtime_module_t runtime,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);

void plugin_basicanim_module_free(plugin_basicanim_module_t mgr);

plugin_basicanim_module_t plugin_basicanim_module_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_basicanim_module_t plugin_basicanim_module_find_nc(gd_app_context_t app, const char * name);

ui_data_mgr_t plugin_basicanim_module_data_mgr(plugin_basicanim_module_t mgr);

gd_app_context_t plugin_basicanim_module_app(plugin_basicanim_module_t mgr);
const char * plugin_basicanim_module_name(plugin_basicanim_module_t mgr);

#ifdef __cplusplus
}
#endif

#endif 

