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
#include "appsvr/account/appsvr_account_module.h"
#include "appsvr_facebook_module_i.h"
#include "appsvr_facebook_permission_i.h"

static void appsvr_facebook_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_appsvr_facebook_module = {
    "appsvr_facebook_module",
    appsvr_facebook_module_clear
};

static struct {
    const char * name; 
    int (*init)(appsvr_facebook_module_t module);
    void (*fini)(appsvr_facebook_module_t module);
} s_auto_reg_products[] = {
    { "backend", appsvr_facebook_backend_init, appsvr_facebook_backend_fini }
    , { "suspend", appsvr_facebook_suspend_monitor_init, appsvr_facebook_suspend_monitor_fini }
    , { "account", appsvr_facebook_login_init, appsvr_facebook_login_fini }
};

appsvr_facebook_module_t
appsvr_facebook_module_create(
    gd_app_context_t app, uint8_t debug,
    appsvr_account_module_t account_module,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    appsvr_facebook_module_t module;
    nm_node_t module_node;
    uint16_t component_pos;
    
    assert(app);

    if (name == NULL) name = "appsvr_facebook_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct appsvr_facebook_module));
    if (module_node == NULL) return NULL;

    module = (appsvr_facebook_module_t)nm_node_data(module_node);

    module->m_app = app; 
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = debug;
    module->m_account_module = account_module;
	module->m_token = NULL;
    module->m_account_adapter = NULL;
    module->m_suspend_monitor = NULL;
    
    module->m_app_env = plugin_app_env_module_find_nc(app, NULL);
    if (module->m_app_env == NULL) {
        CPE_ERROR(em, "appsvr_facebook_module: app env not exist!");
        nm_node_free(module_node);
        return NULL;
    }
    
    mem_buffer_init(&module->m_dump_buffer, alloc);

    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(module->m_em, "appsvr_facebook_module_create: regist product %s fail!", s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }

            mem_buffer_clear(&module->m_dump_buffer);
            nm_node_free(module_node);
            return NULL;
        }
    }

    TAILQ_INIT(&module->m_permissions);

    nm_node_set_type(module_node, &s_nm_node_type_appsvr_facebook_module);

    return module;
}

static void appsvr_facebook_module_clear(nm_node_t node) {
    appsvr_facebook_module_t module = nm_node_data(node);
    uint16_t component_pos;

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

	if (module->m_token) {
		mem_free(module->m_alloc, module->m_token);
		module->m_token = NULL;
	}

    while(!TAILQ_EMPTY(&module->m_permissions)) {
        appsvr_facebook_permission_free(TAILQ_FIRST(&module->m_permissions));
    }

    mem_buffer_clear(&module->m_dump_buffer);
}

gd_app_context_t appsvr_facebook_module_app(appsvr_facebook_module_t module) {
    return module->m_app;
}

void appsvr_facebook_module_free(appsvr_facebook_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_appsvr_facebook_module) return;
    nm_node_free(module_node);
}

appsvr_facebook_module_t
appsvr_facebook_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_facebook_module) return NULL;
    return (appsvr_facebook_module_t)nm_node_data(node);
}

appsvr_facebook_module_t
appsvr_facebook_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "appsvr_facebook_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_facebook_module) return NULL;
    return (appsvr_facebook_module_t)nm_node_data(node);
}

const char * appsvr_facebook_module_name(appsvr_facebook_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

int appsvr_facebook_set_token(appsvr_facebook_module_t module, const char * token) {
    if (module->m_token) {
        mem_free(module->m_alloc, module->m_token);
    }

    if (token) {
        module->m_token = cpe_str_mem_dup(module->m_alloc, token);
        if (module->m_token == NULL) {
            CPE_ERROR(module->m_em, "appsvr_facebook_module: set token: alloc fail!");
            return -1;
        }
    }
    else {
        module->m_token = NULL;
    }

    return 0;
}

EXPORT_DIRECTIVE
int appsvr_facebook_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    appsvr_facebook_module_t appsvr_facebook;
    struct cfg_it permission_it;
    cfg_t permission_cfg;
    
    appsvr_facebook =
        appsvr_facebook_module_create(
            app,
            cfg_get_uint8(cfg, "debug", 0),
            appsvr_account_module_find_nc(app, NULL),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (appsvr_facebook == NULL) return -1;

    cfg_it_init(&permission_it, cfg_find_cfg(cfg, "permissions"));
    while((permission_cfg = cfg_it_next(&permission_it))) {
        const char * str_permission = cfg_as_string(permission_cfg, NULL);
        appsvr_facebook_permission_t permission;
        
        if (str_permission == NULL) {
            APP_CTX_ERROR(app, "appsvr_facebook_module_app_init: permission format error!");
            appsvr_facebook_module_free(appsvr_facebook);
            return -1;
        }

        permission = appsvr_facebook_permission_create(appsvr_facebook, str_permission);
        if (permission == NULL) {
            APP_CTX_ERROR(app, "appsvr_facebook_module_app_init: permission create error!");
            appsvr_facebook_module_free(appsvr_facebook);
            return -1;
        }
    }

    return 0;
}

EXPORT_DIRECTIVE
void appsvr_facebook_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    appsvr_facebook_module_t appsvr_facebook;

    appsvr_facebook = appsvr_facebook_module_find_nc(app, gd_app_module_name(module));
    if (appsvr_facebook) {
        appsvr_facebook_module_free(appsvr_facebook);
    }
}
