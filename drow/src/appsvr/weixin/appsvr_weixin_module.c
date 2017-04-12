#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "gd/net_trans/net_trans_manage.h"
#include "gd/net_trans/net_trans_group.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "appsvr/account/appsvr_account_module.h"
#include "appsvr/payment/appsvr_payment_module.h"
#include "appsvr_weixin_module_i.h"

static void appsvr_weixin_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_appsvr_weixin_module = {
    "appsvr_weixin_module",
    appsvr_weixin_module_clear
};

static struct {
    const char * name; 
    int (*init)(appsvr_weixin_module_t module);
    void (*fini)(appsvr_weixin_module_t module);
} s_auto_reg_products[] = {
    { "backend", appsvr_weixin_backend_init, appsvr_weixin_backend_fini }
    , { "login", appsvr_weixin_login_init, appsvr_weixin_login_fini }
};

appsvr_weixin_module_t
appsvr_weixin_module_create(
    gd_app_context_t app, uint8_t debug,
    appsvr_account_module_t account_module,
    const char * appid, const char * secret, const char * scope,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    appsvr_weixin_module_t module;
    nm_node_t module_node;
    uint16_t component_pos;
    net_trans_manage_t trans_mgr;
    
    assert(app);

    if (name == NULL) name = "appsvr_weixin_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct appsvr_weixin_module));
    if (module_node == NULL) return NULL;

    module = (appsvr_weixin_module_t)nm_node_data(module_node);

    module->m_app = app; 
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = debug;
    module->m_appid = cpe_str_mem_dup(alloc, appid);
    module->m_secret = cpe_str_mem_dup(alloc, secret);
    module->m_scope = cpe_str_mem_dup(alloc, scope);
    module->m_account_module = account_module;

    trans_mgr = net_trans_manage_find_nc(app, NULL);
    if (trans_mgr == NULL) {
        CPE_ERROR(em, "appsvr_weixin_module: net_trans_manage not exist!");
        nm_node_free(module_node);
        return NULL;
    }
    
    module->m_app_env = plugin_app_env_module_find_nc(app, NULL);
    if (module->m_app_env == NULL) {
        CPE_ERROR(em, "appsvr_weixin_module: app env not exist!");
        nm_node_free(module_node);
        return NULL;
    }

    module->m_trans_group = net_trans_group_create(trans_mgr, "weixin");
    if (module->m_trans_group == NULL) {
        CPE_ERROR(em, "appsvr_weixin_module: create trans group fail!");
        nm_node_free(module_node);
        return NULL;
    }
        
    mem_buffer_init(&module->m_dump_buffer, alloc);

    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(module->m_em, "appsvr_weixin_module_create: regist product %s fail!", s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }

            net_trans_group_free(module->m_trans_group);
            mem_buffer_clear(&module->m_dump_buffer);
            nm_node_free(module_node);
            return NULL;
        }
    }

    nm_node_set_type(module_node, &s_nm_node_type_appsvr_weixin_module);

    return module;
}

static void appsvr_weixin_module_clear(nm_node_t node) {
    appsvr_weixin_module_t module = nm_node_data(node);
    uint16_t component_pos;

    assert(module->m_trans_group);
    net_trans_group_free(module->m_trans_group);
    module->m_trans_group = NULL;

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    if (module->m_appid) {
        mem_free(module->m_alloc, module->m_appid);
        module->m_appid = NULL;
    }

    if (module->m_secret) {
        mem_free(module->m_alloc, module->m_secret);
        module->m_secret = NULL;
    }
    
    mem_buffer_clear(&module->m_dump_buffer);
}

gd_app_context_t appsvr_weixin_module_app(appsvr_weixin_module_t module) {
    return module->m_app;
}

void appsvr_weixin_module_free(appsvr_weixin_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_appsvr_weixin_module) return;
    nm_node_free(module_node);
}

appsvr_weixin_module_t
appsvr_weixin_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_weixin_module) return NULL;
    return (appsvr_weixin_module_t)nm_node_data(node);
}

appsvr_weixin_module_t
appsvr_weixin_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "appsvr_weixin_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_weixin_module) return NULL;
    return (appsvr_weixin_module_t)nm_node_data(node);
}

const char * appsvr_weixin_module_name(appsvr_weixin_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

EXPORT_DIRECTIVE
int appsvr_weixin_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    appsvr_weixin_module_t appsvr_weixin;
    const char * appid;
    const char * secret;
    const char * scope;
    
    appid = cfg_get_string(gd_app_cfg(app), "args.wx.appid", NULL);
    if (appid == NULL) {
        APP_CTX_ERROR(app, "weixin: args.wx.appid not configured!");
        return -1;
    }

    secret = cfg_get_string(gd_app_cfg(app), "args.wx.secret", NULL);
    if (secret == NULL) {
        APP_CTX_ERROR(app, "weixin: args.wx.secret not configured!");
        return -1;
    }

    scope = cfg_get_string(gd_app_cfg(app), "args.wx.scope", NULL);
    if (scope == NULL) {
        APP_CTX_ERROR(app, "weixin: args.wx.scope not configured!");
        return -1;
    }
    
    appsvr_weixin =
        appsvr_weixin_module_create(
            app,
            cfg_get_uint8(cfg, "debug", 0),
            appsvr_account_module_find_nc(app, NULL),
            appid, secret, scope,
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (appsvr_weixin == NULL) return -1;
    
    return 0;
}

EXPORT_DIRECTIVE
void appsvr_weixin_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    appsvr_weixin_module_t appsvr_weixin;

    appsvr_weixin = appsvr_weixin_module_find_nc(app, gd_app_module_name(module));
    if (appsvr_weixin) {
        appsvr_weixin_module_free(appsvr_weixin);
    }
}
