#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_library.h"
#include "gd/app/app_log.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "appsvr_package_module_i.h"

static void appsvr_package_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_appsvr_package_module = {
    "appsvr_package_module",
    appsvr_package_module_clear
};

appsvr_package_module_t
appsvr_package_module_create(
    gd_app_context_t app, uint8_t debug,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    appsvr_package_module_t module;
    nm_node_t module_node;
    
    assert(app);

    if (name == NULL) name = "appsvr_package_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct appsvr_package_module));
    if (module_node == NULL) return NULL;

    module = (appsvr_package_module_t)nm_node_data(module_node);

    module->m_app = app; 
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = debug;
    
    nm_node_set_type(module_node, &s_nm_node_type_appsvr_package_module);

    return module;
}

static void appsvr_package_module_clear(nm_node_t node) {
}

gd_app_context_t appsvr_package_module_app(appsvr_package_module_t module) {
    return module->m_app;
}

void appsvr_package_module_free(appsvr_package_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_appsvr_package_module) return;
    nm_node_free(module_node);
}

appsvr_package_module_t
appsvr_package_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_package_module) return NULL;
    return (appsvr_package_module_t)nm_node_data(node);
}

appsvr_package_module_t
appsvr_package_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "appsvr_package_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_package_module) return NULL;
    return (appsvr_package_module_t)nm_node_data(node);
}

const char * appsvr_package_module_name(appsvr_package_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

EXPORT_DIRECTIVE
int appsvr_package_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    appsvr_package_module_t package;
    
    package =
        appsvr_package_module_create(
            app,
            cfg_get_uint8(cfg, "debug", 0),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (package == NULL) return -1;

    
    return 0;
}

EXPORT_DIRECTIVE
void appsvr_package_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    appsvr_package_module_t appsvr_package_module;

    appsvr_package_module = appsvr_package_module_find_nc(app, gd_app_module_name(module));
    if (appsvr_package_module) {
        appsvr_package_module_free(appsvr_package_module);
    }
}
