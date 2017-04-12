#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "render/model/ui_data_mgr.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "render/cache/ui_cache_manager.h"
#include "plugin_pack_module_i.h"

static void plugin_pack_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_plugin_pack_module = {
    "plugin_pack_module",
    plugin_pack_module_clear
};

plugin_pack_module_t
plugin_pack_module_create(
    gd_app_context_t app, ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr, ui_ed_mgr_t ed_mgr,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct plugin_pack_module * module;
    nm_node_t module_node;

    assert(app);

    if (name == NULL) name = "plugin_pack_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_pack_module));
    if (module_node == NULL) return NULL;

    module = (plugin_pack_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = 0;
    module->m_data_mgr = data_mgr;
    module->m_cache_mgr = cache_mgr;
    module->m_ed_mgr = ed_mgr;
    
    mem_buffer_init(&module->m_dump_buffer, module->m_alloc);

    nm_node_set_type(module_node, &s_nm_node_type_plugin_pack_module);

    return module;
}

static void plugin_pack_module_clear(nm_node_t node) {
    plugin_pack_module_t module;

    module = nm_node_data(node);

    mem_buffer_clear(&module->m_dump_buffer);
}

gd_app_context_t plugin_pack_module_app(plugin_pack_module_t module) {
    return module->m_app;
}

void plugin_pack_module_free(plugin_pack_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_plugin_pack_module) return;
    nm_node_free(module_node);
}

plugin_pack_module_t
plugin_pack_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_pack_module) return NULL;
    return (plugin_pack_module_t)nm_node_data(node);
}

plugin_pack_module_t
plugin_pack_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "plugin_pack_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_pack_module) return NULL;
    return (plugin_pack_module_t)nm_node_data(node);
}

const char * plugin_pack_module_name(plugin_pack_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}


EXPORT_DIRECTIVE
int plugin_pack_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_pack_module_t plugin_pack_module;
    ui_data_mgr_t data_mgr;
    ui_cache_manager_t cache_mgr;
    ui_ed_mgr_t ed_mgr;

    data_mgr = ui_data_mgr_find_nc(app, cfg_get_string(cfg, "data-mgr", NULL));
    if (data_mgr == NULL) {
        APP_CTX_ERROR(app, "create plugin_pack_module: data-mgr not exist");
        return -1;
    }

    cache_mgr = ui_cache_manager_find_nc(app, cfg_get_string(cfg, "cache-mgr", NULL));
    if (cache_mgr == NULL) {
        APP_CTX_ERROR(app, "create plugin_pack_module: cache-mgr not exist");
        return -1;
    }

    ed_mgr = ui_ed_mgr_find_nc(app, cfg_get_string(cfg, "ed-mgr", NULL));
    if (ed_mgr == NULL) {
        APP_CTX_ERROR(app, "create plugin_pack_module: ed-mgr not exist");
        return -1;
    }
    
    plugin_pack_module =
        plugin_pack_module_create(
            app, data_mgr, cache_mgr, ed_mgr,
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (plugin_pack_module == NULL) return -1;

    plugin_pack_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (plugin_pack_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_pack_module_name(plugin_pack_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void plugin_pack_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_pack_module_t plugin_pack_module;

    plugin_pack_module = plugin_pack_module_find_nc(app, gd_app_module_name(module));
    if (plugin_pack_module) {
        plugin_pack_module_free(plugin_pack_module);
    }
}
