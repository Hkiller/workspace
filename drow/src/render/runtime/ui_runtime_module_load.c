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
#include "render/cache/ui_cache_color.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render_cmd.h"
#include "ui_runtime_module_i.h"
#include "ui_runtime_render_obj_i.h"
#include "ui_runtime_render_i.h"

EXPORT_DIRECTIVE
int ui_runtime_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    ui_runtime_module_t ui_runtime_module;
    ui_data_mgr_t data_mgr;
    ui_cache_manager_t cache_mgr;
    const char * str_value;
    uint64_t buf_size;
    ui_runtime_render_t render;

    data_mgr = ui_data_mgr_find_nc(app, cfg_get_string(cfg, "data-mgr", NULL));
    if (data_mgr == NULL) {
        APP_CTX_ERROR(app, "create %s: data-mgr not exist", gd_app_module_name(module));
        return -1;
    }
    
    cache_mgr = ui_cache_manager_find_nc(app, cfg_get_string(cfg, "cache-mgr", NULL));
    if (cache_mgr == NULL) {
        APP_CTX_ERROR(app, "create %s: cache-mgr not exist", gd_app_module_name(module));
        return -1;
    }

    ui_runtime_module =
        ui_runtime_module_create(
            app, gd_app_alloc(app), data_mgr, cache_mgr,
            gd_app_module_name(module), gd_app_em(app));
    if (ui_runtime_module == NULL) return -1;

    ui_runtime_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    str_value = cfg_get_string(cfg, "render.op-buf-size", NULL);
    if (str_value == NULL) {
        APP_CTX_ERROR(app, "create %s: render: op-buf-size not configured", gd_app_module_name(module));
        ui_runtime_module_free(ui_runtime_module);
        return -1;
    }
    
    if (cpe_str_parse_byte_size(&buf_size, str_value) != 0) {
        APP_CTX_ERROR(app, "create %s: render: op-buf-size '%s' format error", gd_app_module_name(module), str_value);
        ui_runtime_module_free(ui_runtime_module);
        return -1;
    }

    render = ui_runtime_render_create(ui_runtime_module, buf_size);
    if (render == NULL) {
        APP_CTX_ERROR(app, "create %s: render: create context fail", gd_app_module_name(module));
        ui_runtime_module_free(ui_runtime_module);
        return -1;
    }

    if ((str_value = cfg_get_string(cfg, "render.clear-color", NULL))) {
        ui_color c;
        if (ui_cache_find_color(ui_runtime_module->m_cache_mgr, str_value, &c) != 0) {
            APP_CTX_ERROR(app, "create %s: render: clear-color %s unknown", gd_app_module_name(module), str_value);
            ui_runtime_module_free(ui_runtime_module);
            return -1;
        }
        render->m_clear_color = c;
    }
    
    if (ui_runtime_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            ui_runtime_module_name(ui_runtime_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void ui_runtime_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    ui_runtime_module_t ui_runtime_module;

    ui_runtime_module = ui_runtime_module_find_nc(app, gd_app_module_name(module));
    if (ui_runtime_module) {
        ui_runtime_module_free(ui_runtime_module);
    }
}
