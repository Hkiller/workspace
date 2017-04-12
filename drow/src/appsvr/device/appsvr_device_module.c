#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "plugin/app_env/plugin_app_env_executor.h"
#include "appsvr_device_module_i.h"
#include "appsvr_device_executor_i.h"

extern char g_metalib_appsvr_device[];
static void appsvr_device_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_appsvr_device_module = {
    "appsvr_device_module",
    appsvr_device_module_clear
};

static struct {
    const char * name; 
    int (*init)(appsvr_device_module_t module);
    void (*fini)(appsvr_device_module_t module);
} s_auto_reg_products[] = {
    { "backend", appsvr_device_backend_init, appsvr_device_backend_fini }
};

static struct plugin_app_env_executor_def s_executor_defs[] = {
    { "appsvr_device_query_path", plugin_app_env_executor_sync, { (void*)appsvr_device_query_path } }
    , { "appsvr_device_query_info", plugin_app_env_executor_sync, { (void*)appsvr_device_query_info } }
    , { "appsvr_device_query_network", plugin_app_env_executor_sync, { (void*)appsvr_device_query_network_state } }
} ;

appsvr_device_module_t
appsvr_device_module_create(
    gd_app_context_t app, uint8_t debug,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    appsvr_device_module_t module;
    nm_node_t module_node;
    uint16_t component_pos;
    
    assert(app);

    if (name == NULL) name = "appsvr_device_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct appsvr_device_module));
    if (module_node == NULL) return NULL;

    module = (appsvr_device_module_t)nm_node_data(module_node);

    module->m_app = app; 
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = debug;

    module->m_app_env = plugin_app_env_module_find_nc(app, NULL);
    if (module->m_app_env == NULL) {
        CPE_ERROR(em, "appsvr_device_module: app env not exist!");
        nm_node_free(module_node);
        return NULL;
    }

    module->m_meta_device_info = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_appsvr_device, "appsvr_device_info");
    assert(module->m_meta_device_info);

    module->m_meta_path_info = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_appsvr_device, "appsvr_device_path");
    assert(module->m_meta_path_info);

    module->m_meta_network_info = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_appsvr_device, "appsvr_device_network_info");
    assert(module->m_meta_network_info);
    
    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(module->m_em, "appsvr_device_module_create: regist product %s fail!", s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }
            nm_node_free(module_node);
            return NULL;
        }
    }

    if (plugin_app_env_executor_bulck_create(
            module->m_app_env, (LPDRMETALIB)g_metalib_appsvr_device, module,
            s_executor_defs, (uint8_t)CPE_ARRAY_SIZE(s_executor_defs)) != 0)
    {
        CPE_ERROR(module->m_em, "appsvr_device_module: install executors fail!");
        for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
            s_auto_reg_products[component_pos - 1].fini(module);
        }
        nm_node_free(module_node);
        return NULL;
    }

    mem_buffer_init(&module->m_dump_buffer, alloc);

    nm_node_set_type(module_node, &s_nm_node_type_appsvr_device_module);

    return module;
}

static void appsvr_device_module_clear(nm_node_t node) {
    appsvr_device_module_t module = nm_node_data(node);
    uint16_t component_pos;

    plugin_app_env_executor_free_by_ctx(module->m_app_env, module);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    mem_buffer_clear(&module->m_dump_buffer);
}

gd_app_context_t appsvr_device_module_app(appsvr_device_module_t module) {
    return module->m_app;
}

void appsvr_device_module_free(appsvr_device_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_appsvr_device_module) return;
    nm_node_free(module_node);
}

appsvr_device_module_t
appsvr_device_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_device_module) return NULL;
    return (appsvr_device_module_t)nm_node_data(node);
}

appsvr_device_module_t
appsvr_device_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "appsvr_device_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_device_module) return NULL;
    return (appsvr_device_module_t)nm_node_data(node);
}

const char * appsvr_device_module_name(appsvr_device_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

EXPORT_DIRECTIVE
int appsvr_device_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    appsvr_device_module_t appsvr_device;
    
    appsvr_device =
        appsvr_device_module_create(
            app,
            cfg_get_uint8(cfg, "debug", 0),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (appsvr_device == NULL) return -1;

    return 0;
}

EXPORT_DIRECTIVE
void appsvr_device_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    appsvr_device_module_t appsvr_device;

    appsvr_device = appsvr_device_module_find_nc(app, gd_app_module_name(module));
    if (appsvr_device) {
        appsvr_device_module_free(appsvr_device);
    }
}
