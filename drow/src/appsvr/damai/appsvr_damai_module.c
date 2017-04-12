#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_store/dr_store.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "appsvr/account/appsvr_account_module.h"
#include "appsvr/payment/appsvr_payment_module.h"
#include "appsvr_damai_module_i.h"

static void appsvr_damai_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_appsvr_damai_module = {
    "appsvr_damai_module",
    appsvr_damai_module_clear
};

static struct {
    const char * name; 
    int (*init)(appsvr_damai_module_t module);
    void (*fini)(appsvr_damai_module_t module);
} s_auto_reg_products[] = {
    { "backend", appsvr_damai_backend_init, appsvr_damai_backend_fini }
    , { "suspend-monitor", appsvr_damai_suspend_monitor_init, appsvr_damai_suspend_monitor_fini }
    , { "set-userinfo", appsvr_damai_set_userinfo_init, appsvr_damai_set_userinfo_fini }
};

appsvr_damai_module_t
appsvr_damai_module_create(
    gd_app_context_t app, uint8_t debug,
    appsvr_account_module_t account_module,
    appsvr_payment_module_t payment_module,
    LPDRMETA userinfo_source_meta,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    appsvr_damai_module_t module;
    nm_node_t module_node;
    uint16_t component_pos;
    
    assert(app);

    if (name == NULL) name = "appsvr_damai_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct appsvr_damai_module));
    if (module_node == NULL) return NULL;

    module = (appsvr_damai_module_t)nm_node_data(module_node);

    module->m_app = app; 
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = debug;
    module->m_payment_module = payment_module;
    module->m_account_module = account_module;
    module->m_suspend_monitor = NULL;
    module->m_set_userinfo_executor = NULL;
    module->m_backend = NULL;

    module->m_userinfo_source_meta = userinfo_source_meta;
    module->m_userinfo_service = NULL;
    module->m_userinfo_role = NULL;
    module->m_userinfo_grade = NULL;
    
    module->m_app_env = plugin_app_env_module_find_nc(app, NULL);
    if (module->m_app_env == NULL) {
        CPE_ERROR(em, "appsvr_damai_module: app env not exist!");
        nm_node_free(module_node);
        return NULL;
    }

    module->m_computer = xcomputer_create(alloc, em);
    if (module->m_computer == NULL) {
        CPE_ERROR(em, "appsvr_damai_module: create computer fail!");
        nm_node_free(module_node);
        return NULL;
    }
    
    mem_buffer_init(&module->m_dump_buffer, alloc);

    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(module->m_em, "appsvr_damai_module_create: regist product %s fail!", s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }

            xcomputer_free(module->m_computer);            
            mem_buffer_clear(&module->m_dump_buffer);
            nm_node_free(module_node);
            return NULL;
        }
    }

    nm_node_set_type(module_node, &s_nm_node_type_appsvr_damai_module);

    return module;
}

static void appsvr_damai_module_clear(nm_node_t node) {
    appsvr_damai_module_t module = nm_node_data(node);
    uint16_t component_pos;

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    xcomputer_free(module->m_computer);
    module->m_computer = NULL;
    
    if (module->m_userinfo_service) {
        mem_free(module->m_alloc, module->m_userinfo_service);
        module->m_userinfo_service = NULL;
    }

    if (module->m_userinfo_role) {
        mem_free(module->m_alloc, module->m_userinfo_role);
        module->m_userinfo_role = NULL;
    }

    if (module->m_userinfo_grade) {
        mem_free(module->m_alloc, module->m_userinfo_grade);
        module->m_userinfo_grade = NULL;
    }

    mem_buffer_clear(&module->m_dump_buffer);
}

gd_app_context_t appsvr_damai_module_app(appsvr_damai_module_t module) {
    return module->m_app;
}

void appsvr_damai_module_free(appsvr_damai_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_appsvr_damai_module) return;
    nm_node_free(module_node);
}

appsvr_damai_module_t
appsvr_damai_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_damai_module) return NULL;
    return (appsvr_damai_module_t)nm_node_data(node);
}

appsvr_damai_module_t
appsvr_damai_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "appsvr_damai_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_damai_module) return NULL;
    return (appsvr_damai_module_t)nm_node_data(node);
}

const char * appsvr_damai_module_name(appsvr_damai_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

EXPORT_DIRECTIVE
int appsvr_damai_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    appsvr_damai_module_t appsvr_damai;
    dr_store_manage_t store_manage;
    const char * str_value;
    const char * userinfo_service;
    const char * userinfo_role;
    const char * userinfo_grade;
    LPDRMETA userinfo_source_meta;
    
    store_manage = dr_store_manage_find_nc(app, NULL);
    if (store_manage == NULL) {
        APP_CTX_ERROR(app, "appsvr_damai_module_app_init: store-manager not exist!");
        return -1;
    }

    str_value = cfg_get_string(cfg, "user-info.source-meta", NULL);
    if (str_value == NULL) {
        APP_CTX_ERROR(app, "appsvr_damai_module_app_init: user-info.source-meta not configured!");
        return -1;
    }

    userinfo_source_meta = dr_store_manage_find_meta(store_manage, str_value);
    if (userinfo_source_meta == NULL) {
        APP_CTX_ERROR(app, "appsvr_damai_module_app_init: get userinfo source meta %s fail!", str_value);
        return -1;
    }            

    userinfo_service = cfg_get_string(cfg, "user-info.attrs.service", NULL);
    if (userinfo_service == NULL) {
        APP_CTX_ERROR(app, "appsvr_damai_module_app_init: user-info.attrs.service not configured!");
        return -1;
    }
    
    userinfo_role = cfg_get_string(cfg, "user-info.attrs.role", NULL);
    if (userinfo_role == NULL) {
        APP_CTX_ERROR(app, "appsvr_damai_module_app_init: user-info.attrs.role not configured!");
        return -1;
    }

    userinfo_grade = cfg_get_string(cfg, "user-info.attrs.grade", NULL);
    if (userinfo_grade == NULL) {
        APP_CTX_ERROR(app, "appsvr_damai_module_app_init: user-info.attrs.grade not configured!");
        return -1;
    }
    
    appsvr_damai =
        appsvr_damai_module_create(
            app,
            cfg_get_uint8(cfg, "debug", 0),
            appsvr_account_module_find_nc(app, NULL),
            appsvr_payment_module_find_nc(app, NULL),
            userinfo_source_meta,
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (appsvr_damai == NULL) return -1;

    appsvr_damai->m_userinfo_service = cpe_str_mem_dup(appsvr_damai->m_alloc, userinfo_service);
    appsvr_damai->m_userinfo_role = cpe_str_mem_dup(appsvr_damai->m_alloc, userinfo_role);
    appsvr_damai->m_userinfo_grade = cpe_str_mem_dup(appsvr_damai->m_alloc, userinfo_grade);

    return 0;
}

EXPORT_DIRECTIVE
void appsvr_damai_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    appsvr_damai_module_t appsvr_damai;

    appsvr_damai = appsvr_damai_module_find_nc(app, gd_app_module_name(module));
    if (appsvr_damai) {
        appsvr_damai_module_free(appsvr_damai);
    }
}
