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
#include "appsvr_account_module_i.h"
#include "appsvr_account_executor.h"
#include "appsvr_account_adapter_i.h"

extern char g_metalib_appsvr_account[];
static void appsvr_account_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_appsvr_account_module = {
    "appsvr_account_module",
    appsvr_account_module_clear
};

static struct {
    const char * name; 
    int (*init)(appsvr_account_module_t module);
    void (*fini)(appsvr_account_module_t module);
} s_auto_reg_products[] = {
    { "executor", appsvr_account_executor_init, appsvr_account_executor_fini }    
};

appsvr_account_module_t
appsvr_account_module_create(
    gd_app_context_t app, uint8_t debug,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    appsvr_account_module_t module;
    nm_node_t module_node;
    uint16_t component_pos;
    
    assert(app);

    if (name == NULL) name = "appsvr_account_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct appsvr_account_module));
    if (module_node == NULL) return NULL;

    module = (appsvr_account_module_t)nm_node_data(module_node);

    module->m_app = app; 
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = debug;
    module->m_adapter_count = 0;
    
    module->m_runing_adapter = NULL;
    module->m_runing_id = 0;

    module->m_meta_req_login = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_appsvr_account, "appsvr_account_login");
    assert(module->m_meta_req_login);
    module->m_meta_res_login = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_appsvr_account, "appsvr_account_login_result");
    assert(module->m_meta_res_login);

    module->m_meta_req_relogin = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_appsvr_account, "appsvr_account_relogin");
    assert(module->m_meta_req_relogin);

    module->m_meta_req_query_services = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_appsvr_account, "appsvr_account_query_services");
    assert(module->m_meta_req_query_services);
    module->m_meta_res_query_services = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_appsvr_account, "appsvr_account_service_list");
    assert(module->m_meta_res_query_services);
    
    module->m_app_env = plugin_app_env_module_find_nc(app, NULL);
    if (module->m_app_env == NULL) {
        CPE_ERROR(em, "appsvr_account_module: app env not exist!");
        nm_node_free(module_node);
        return NULL;
    }

    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(module->m_em, "appsvr_account_module_create: regist product %s fail!", s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }
            nm_node_free(module_node);
            return NULL;
        }
    }

    TAILQ_INIT(&module->m_adapters);
    mem_buffer_init(&module->m_dump_buffer, alloc);

    nm_node_set_type(module_node, &s_nm_node_type_appsvr_account_module);

    return module;
}

static void appsvr_account_module_clear(nm_node_t node) {
    appsvr_account_module_t module = nm_node_data(node);
    uint16_t component_pos;

    while(!TAILQ_EMPTY(&module->m_adapters)) {
        appsvr_account_adapter_free(TAILQ_FIRST(&module->m_adapters));
    }

    assert(module->m_runing_adapter == NULL);
    assert(module->m_runing_id == 0);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    mem_buffer_clear(&module->m_dump_buffer);
}

gd_app_context_t appsvr_account_module_app(appsvr_account_module_t module) {
    return module->m_app;
}

void appsvr_account_module_free(appsvr_account_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_appsvr_account_module) return;
    nm_node_free(module_node);
}

appsvr_account_module_t
appsvr_account_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_account_module) return NULL;
    return (appsvr_account_module_t)nm_node_data(node);
}

appsvr_account_module_t
appsvr_account_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "appsvr_account_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_account_module) return NULL;
    return (appsvr_account_module_t)nm_node_data(node);
}

const char * appsvr_account_module_name(appsvr_account_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

EXPORT_DIRECTIVE
int appsvr_account_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    appsvr_account_module_t account;
    struct cfg_it adapters_it;
    cfg_t adapter_cfg;
    
    account =
        appsvr_account_module_create(
            app,
            cfg_get_uint8(cfg, "debug", 0),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (account == NULL) return -1;

    cfg_it_init(&adapters_it, cfg_find_cfg(gd_app_cfg(app), "account"));
    while((adapter_cfg = cfg_it_next(&adapters_it))) {
        char type_buf[64];
        const char * service_name;
        appsvr_account_adapter_creation_fun_t creator;

        service_name = cfg_name(adapter_cfg);
        if (service_name == NULL) {
            APP_CTX_ERROR(app, "account: load: read service type error");
            appsvr_account_module_free(account);
            return -1;
        }

        snprintf(type_buf, sizeof(type_buf), "appsvr_account_adapter_%s_create", service_name);
        creator = gd_app_lib_sym(NULL, type_buf, account->m_em);
        if (creator == NULL) {
            APP_CTX_INFO(app, "account: load: service %s not exist, ignore", service_name);
            continue;
        }

        if (creator(account, adapter_cfg, account->m_alloc, account->m_em) == NULL) {
            APP_CTX_ERROR(app, "account: load: service %s create fail!", service_name);
            appsvr_account_module_free(account);
            return -1;
        }

        APP_CTX_INFO(app, "account: load: service %s load success", service_name);
    }
    
    return 0;
}

EXPORT_DIRECTIVE
void appsvr_account_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    appsvr_account_module_t appsvr_account_module;

    appsvr_account_module = appsvr_account_module_find_nc(app, gd_app_module_name(module));
    if (appsvr_account_module) {
        appsvr_account_module_free(appsvr_account_module);
    }
}
