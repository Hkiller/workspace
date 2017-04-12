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
#include "appsvr/payment/appsvr_payment_module.h"
#include "appsvr_cmccpay_module_i.h"

static void appsvr_cmccpay_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_appsvr_cmccpay_module = {
    "appsvr_cmccpay_module",
    appsvr_cmccpay_module_clear
};

static struct {
    const char * name; 
    int (*init)(appsvr_cmccpay_module_t module);
    void (*fini)(appsvr_cmccpay_module_t module);
} s_auto_reg_products[] = {
    { "backend", appsvr_cmccpay_backend_init, appsvr_cmccpay_backend_fini }
    , { "monitor-stop", appsvr_cmccpay_monitor_stop_init, appsvr_cmccpay_monitor_stop_fini }
    , { "monitor-more", appsvr_cmccpay_monitor_more_init, appsvr_cmccpay_monitor_more_fini }
};

appsvr_cmccpay_module_t
appsvr_cmccpay_module_create(
    gd_app_context_t app, uint8_t debug,
    appsvr_account_module_t account_module,
    appsvr_payment_module_t payment_module,
    mem_allocrator_t alloc, const char * name, error_monitor_t em,const char * more_game_evt_name)
{
    appsvr_cmccpay_module_t module;
    nm_node_t module_node;
    uint16_t component_pos;
    
    assert(app);

    if (name == NULL) name = "appsvr_cmccpay_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct appsvr_cmccpay_module));
    if (module_node == NULL) return NULL;

    module = (appsvr_cmccpay_module_t)nm_node_data(module_node);

    module->m_app = app; 
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = debug;
    module->m_payment_module = payment_module;
    module->m_stop_monitor = NULL;


    if (more_game_evt_name) {
        module->m_more_game_evt_name = cpe_str_mem_dup(module->m_alloc, more_game_evt_name);
        if (module->m_more_game_evt_name == NULL) {
            CPE_ERROR(em, "appsvr_cmccpay_module: m_more_game_evt_name = null");
            nm_node_free(module_node);
            return NULL;
        }
    }

    module->m_app_env = plugin_app_env_module_find_nc(app, NULL);
    if (module->m_app_env == NULL) {
        CPE_ERROR(em, "appsvr_cmccpay_module: app env not exist!");
        nm_node_free(module_node);
        return NULL;
    }
    
    mem_buffer_init(&module->m_dump_buffer, alloc);

    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(module->m_em, "appsvr_cmccpay_module_create: regist product %s fail!", s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }

            mem_buffer_clear(&module->m_dump_buffer);
            nm_node_free(module_node);
            return NULL;
        }
    }

    nm_node_set_type(module_node, &s_nm_node_type_appsvr_cmccpay_module);

    return module;
}

static void appsvr_cmccpay_module_clear(nm_node_t node) {
    appsvr_cmccpay_module_t module = nm_node_data(node);
    uint16_t component_pos;

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    if (module->m_more_game_evt_name) {
        mem_free(module->m_alloc, module->m_more_game_evt_name);
        module->m_more_game_evt_name = NULL;
    }

    mem_buffer_clear(&module->m_dump_buffer);
}

gd_app_context_t appsvr_cmccpay_module_app(appsvr_cmccpay_module_t module) {
    return module->m_app;
}

void appsvr_cmccpay_module_free(appsvr_cmccpay_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_appsvr_cmccpay_module) return;
    nm_node_free(module_node);
}

appsvr_cmccpay_module_t
appsvr_cmccpay_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_cmccpay_module) return NULL;
    return (appsvr_cmccpay_module_t)nm_node_data(node);
}

appsvr_cmccpay_module_t
appsvr_cmccpay_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "appsvr_cmccpay_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_cmccpay_module) return NULL;
    return (appsvr_cmccpay_module_t)nm_node_data(node);
}

const char * appsvr_cmccpay_module_name(appsvr_cmccpay_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

EXPORT_DIRECTIVE
int appsvr_cmccpay_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    appsvr_cmccpay_module_t appsvr_cmccpay;
    
    appsvr_cmccpay =
        appsvr_cmccpay_module_create(
            app,
            cfg_get_uint8(cfg, "debug", 0),
            appsvr_account_module_find_nc(app, NULL),
            appsvr_payment_module_find_nc(app, NULL),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app),
            cfg_get_string(cfg, "more-game-notification", NULL));
    if (appsvr_cmccpay == NULL) return -1;

    return 0;
}

EXPORT_DIRECTIVE
void appsvr_cmccpay_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    appsvr_cmccpay_module_t appsvr_cmccpay;

    appsvr_cmccpay = appsvr_cmccpay_module_find_nc(app, gd_app_module_name(module));
    if (appsvr_cmccpay) {
        appsvr_cmccpay_module_free(appsvr_cmccpay);
    }
}