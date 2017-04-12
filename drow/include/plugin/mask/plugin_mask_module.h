#ifndef DROW_PLUGIN_MASK_MODULE_H
#define DROW_PLUGIN_MASK_MODULE_H
#include "gd/app/app_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "cpe/dr/dr_types.h"
#include "render/model/ui_model_types.h"
#include "render/runtime/ui_runtime_types.h"
#include "plugin_mask_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_mask_module_t
plugin_mask_module_create(
    gd_app_context_t app, ui_data_mgr_t data_mgr, ui_runtime_module_t runtime,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);

void plugin_mask_module_free(plugin_mask_module_t mgr);

plugin_mask_module_t plugin_mask_module_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_mask_module_t plugin_mask_module_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t plugin_mask_module_app(plugin_mask_module_t mgr);
const char * plugin_mask_module_name(plugin_mask_module_t mgr);

ui_data_mgr_t plugin_mask_module_data_mgr(plugin_mask_module_t mgr);

void plugin_mask_module_install_bin_loader(plugin_mask_module_t mgr);
void plugin_mask_module_install_bin_saver(plugin_mask_module_t mgr);

#ifdef __cplusplus
}
#endif

#endif 

