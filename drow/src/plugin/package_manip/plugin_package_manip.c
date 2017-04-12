#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_calc.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin_package_manip_i.h"
#include "plugin_package_manip_res_collector_i.h"
#include "plugin_package_manip_src_convertor_i.h"

static void plugin_package_manip_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_plugin_package_manip = {
    "plugin_package_manip",
    plugin_package_manip_clear
};

plugin_package_manip_t
plugin_package_manip_create(
    gd_app_context_t app, ui_ed_mgr_t ed_mgr, plugin_package_module_t package_module, 
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct plugin_package_manip * module;
    nm_node_t module_node;

    assert(app);

    if (name == NULL) name = "plugin_package_manip";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_package_manip));
    if (module_node == NULL) return NULL;

    module = (plugin_package_manip_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = 0;
    module->m_ed_mgr = ed_mgr;
    module->m_package_module = package_module;
    TAILQ_INIT(&module->m_res_collectors);
    TAILQ_INIT(&module->m_src_convertors);

    module->m_computer = xcomputer_create(alloc, em);
    if (module->m_computer == NULL) {
        CPE_ERROR(em, "plugin_package_manip_create: create xcomputer fail!");
        nm_node_free(module_node);
        return NULL;
    }
    
    nm_node_set_type(module_node, &s_nm_node_type_plugin_package_manip);

    if (plugin_package_manip_create_res_collector_src(module) != 0
        || plugin_package_manip_create_res_collector_res(module) != 0
        || plugin_package_manip_create_res_collector_package(module) != 0
        || plugin_package_manip_create_res_collector_region(module) != 0
        || plugin_package_manip_create_res_collector_extern_shared(module) != 0
        )
    {
        plugin_package_manip_free(module);
        return NULL;
    }
    
    return module;
}

static void plugin_package_manip_clear(nm_node_t node) {
    plugin_package_manip_t module;

    module = nm_node_data(node);
    
    while(!TAILQ_EMPTY(&module->m_res_collectors)) {
        plugin_package_manip_res_collector_free(TAILQ_FIRST(&module->m_res_collectors));
    }

    while(!TAILQ_EMPTY(&module->m_src_convertors)) {
        plugin_package_manip_src_convertor_free(TAILQ_FIRST(&module->m_src_convertors));
    }

    xcomputer_free(module->m_computer);
}

gd_app_context_t plugin_package_manip_app(plugin_package_manip_t module) {
    return module->m_app;
}

void plugin_package_manip_free(plugin_package_manip_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_plugin_package_manip) return;
    nm_node_free(module_node);
}

plugin_package_manip_t
plugin_package_manip_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_package_manip) return NULL;
    return (plugin_package_manip_t)nm_node_data(node);
}

plugin_package_manip_t
plugin_package_manip_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "plugin_package_manip";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_package_manip) return NULL;
    return (plugin_package_manip_t)nm_node_data(node);
}

const char * plugin_package_manip_name(plugin_package_manip_t module) {
    return nm_node_name(nm_node_from_data(module));
}

xcomputer_t plugin_package_manip_computer(plugin_package_manip_t manip) {
    return manip->m_computer;
}

const char * plugin_package_manip_calc(plugin_package_manip_t manip, mem_buffer_t buffer, const char * def, cfg_calc_context_t args) {
    if (def[0] == ':') {
        return cfg_try_calc_str(manip->m_computer, buffer, def + 1, args, manip->m_em);
    }
    else {
        return def;
    }
}

EXPORT_DIRECTIVE
int plugin_package_manip_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_package_module_t package_module;
    plugin_package_manip_t plugin_package_manip;

    package_module = plugin_package_module_find_nc(app, cfg_get_string(cfg, "package-module", NULL));
    if (package_module == NULL) {
        APP_CTX_ERROR(app, "create plugin_package_manip: package-module not exist");
        return -1;
    }

    plugin_package_manip =
        plugin_package_manip_create(
            app,
            ui_ed_mgr_find_nc(app, NULL),
            package_module,
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (plugin_package_manip == NULL) return -1;

    plugin_package_manip->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (plugin_package_manip->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_package_manip_name(plugin_package_manip));
    }

    return 0;
}

EXPORT_DIRECTIVE
void plugin_package_manip_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_package_manip_t plugin_package_manip;

    plugin_package_manip = plugin_package_manip_find_nc(app, gd_app_module_name(module));
    if (plugin_package_manip) {
        plugin_package_manip_free(plugin_package_manip);
    }
}
