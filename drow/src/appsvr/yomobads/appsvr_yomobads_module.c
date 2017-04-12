#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "appsvr/ad/appsvr_ad_module.h"
#include "appsvr/ad/appsvr_ad_adapter.h"
#include "appsvr/ad/appsvr_ad_action.h"
#include "appsvr_yomobads_module_i.h"

extern char g_metalib_appsvr_sdk[];
static void appsvr_yomobads_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_appsvr_yomobads_module = {
    "appsvr_yomobads_module",
    appsvr_yomobads_module_clear
};

static struct {
    const char * name; 
    int (*init)(appsvr_yomobads_module_t module);
    void (*fini)(appsvr_yomobads_module_t module);
} s_auto_reg_products[] = {
    { "backend", appsvr_yomobads_backend_init, appsvr_yomobads_backend_fini }
    , { "ad", appsvr_yomobads_ad_init, appsvr_yomobads_ad_fini }
    , { "monitor-suspend", appsvr_yomobads_suspend_monitor_init, appsvr_yomobads_suspend_monitor_fini }
};

appsvr_yomobads_module_t
appsvr_yomobads_module_create(
    gd_app_context_t app, uint8_t debug,
    appsvr_ad_module_t ad_module,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    appsvr_yomobads_module_t module;
    nm_node_t module_node;
    uint16_t component_pos;
    
    assert(app);

    if (name == NULL) name = "appsvr_yomobads_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct appsvr_yomobads_module));
    if (module_node == NULL) return NULL;

    module = (appsvr_yomobads_module_t)nm_node_data(module_node);

    module->m_app = app; 
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = debug;
    module->m_ad_module = ad_module;
    module->m_pause_monitor = NULL;
    module->m_suspend_monitor = NULL;
    module->m_app_id = NULL;
    /*ad*/
    module->m_ad_adapter = NULL;
    module->m_request_id = 0;

    module->m_app_env = plugin_app_env_module_find_nc(app, NULL);
    if (module->m_app_env == NULL) {
        CPE_ERROR(em, "appsvr_yomobads_module: app env not exist!");
        nm_node_free(module_node);
        return NULL;
    }

    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(module->m_em, "appsvr_yomobads_module_create: regist product %s fail!", s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }

            nm_node_free(module_node);
            return NULL;
        }
    }

    nm_node_set_type(module_node, &s_nm_node_type_appsvr_yomobads_module);

    return module;
}

static void appsvr_yomobads_module_clear(nm_node_t node) {
    appsvr_yomobads_module_t module = nm_node_data(node);
    uint16_t component_pos;
    
    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }
    
    if (module->m_app_id) {
        mem_free(module->m_alloc, module->m_app_id);
        module->m_app_id = NULL;
    }
}

gd_app_context_t appsvr_yomobads_module_app(appsvr_yomobads_module_t module) {
    return module->m_app;
}

void appsvr_yomobads_module_free(appsvr_yomobads_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_appsvr_yomobads_module) return;
    nm_node_free(module_node);
}

appsvr_yomobads_module_t
appsvr_yomobads_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_yomobads_module) return NULL;
    return (appsvr_yomobads_module_t)nm_node_data(node);
}

appsvr_yomobads_module_t
appsvr_yomobads_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "appsvr_yomobads_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_yomobads_module) return NULL;
    return (appsvr_yomobads_module_t)nm_node_data(node);
}

const char * appsvr_yomobads_module_name(appsvr_yomobads_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

int appsvr_yomobads_module_set_app_id(appsvr_yomobads_module_t module, const char * app_id) {
    if (app_id == NULL) {
        CPE_ERROR(module->m_em, "iapppay: app id can`t set to NULL");
        return -1;
    }
    
    if (module->m_app_id) {
        mem_free(module->m_alloc, module->m_app_id);
    }
    
    module->m_app_id = cpe_str_mem_dup(module->m_alloc, app_id);
    return module->m_app_id ? 0 : -1;
}


EXPORT_DIRECTIVE
int appsvr_yomobads_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    appsvr_yomobads_module_t appsvr_yomobads;
    appsvr_yomobads =
        appsvr_yomobads_module_create(
            app,
            cfg_get_uint8(cfg, "debug", 0),
            appsvr_ad_module_find_nc(app, NULL),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (appsvr_yomobads == NULL) return -1;

    if (appsvr_yomobads_read_adaction_data(appsvr_yomobads,cfg)) {
        appsvr_yomobads_module_free(appsvr_yomobads);
        return -1;
    }

    return 0;
}

EXPORT_DIRECTIVE
void appsvr_yomobads_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    appsvr_yomobads_module_t appsvr_yomobads;

    appsvr_yomobads = appsvr_yomobads_module_find_nc(app, gd_app_module_name(module));
    if (appsvr_yomobads) {
        appsvr_yomobads_module_free(appsvr_yomobads);
    }
}
