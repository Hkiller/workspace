#ifndef DROW_PLUGIN_REMOTEANIM_MODULE_H
#define DROW_PLUGIN_REMOTEANIM_MODULE_H
#include "gd/app/app_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "cpe/dr/dr_types.h"
#include "plugin_remoteanim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_remoteanim_module_t
plugin_remoteanim_module_create(
    gd_app_context_t app, net_trans_manage_t trans_mgr, ui_runtime_module_t runtime,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);

void plugin_remoteanim_module_free(plugin_remoteanim_module_t mgr);

plugin_remoteanim_module_t plugin_remoteanim_module_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_remoteanim_module_t plugin_remoteanim_module_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t plugin_remoteanim_module_app(plugin_remoteanim_module_t mgr);
const char * plugin_remoteanim_module_name(plugin_remoteanim_module_t mgr);

#ifdef __cplusplus
}
#endif

#endif 

