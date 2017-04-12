#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_log.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_mgr.h"
#include "render/cache/ui_cache_manager.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin_swf_module_i.hpp"

extern "C"
EXPORT_DIRECTIVE
int plugin_swf_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_swf_module_t plugin_swf_module;
    ui_data_mgr_t data_mgr;

    data_mgr = ui_data_mgr_find_nc(app, cfg_get_string(cfg, "data-mgr", NULL));
    if (data_mgr == NULL) {
        APP_CTX_ERROR(app, "create %s: data-mgr not exist", gd_app_module_name(module));
        return -1;
    }

    plugin_swf_module =
        plugin_swf_module_create(
            app, gd_app_alloc(app), data_mgr,
            ui_cache_manager_find_nc(app, cfg_get_string(cfg, "cache-mgr", NULL)),
            ui_runtime_module_find_nc(app, cfg_get_string(cfg, "runtime", NULL)),
            cfg_get_int32(cfg, "debug", 0), gd_app_module_name(module), gd_app_em(app));
    if (plugin_swf_module == NULL) return -1;
    
    if (plugin_swf_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_swf_module_name(plugin_swf_module));
    }

    return 0;
}

extern "C"
EXPORT_DIRECTIVE
void plugin_swf_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_swf_module_t plugin_swf_module;

    plugin_swf_module = plugin_swf_module_find_nc(app, gd_app_module_name(module));
    if (plugin_swf_module) {
        plugin_swf_module_free(plugin_swf_module);
    }
}
