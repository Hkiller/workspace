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
#include "gd/timer/timer_manage.h"
#include "gd/app_attr/app_attr_module.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "appsvr/account/appsvr_account_module.h"
#include "appsvr/payment/appsvr_payment_module.h"
#include "appsvr/payment/appsvr_payment_adapter.h"
#include "appsvr_haoxin_module_i.h"

extern char g_metalib_appsvr_haoxin[];

extern char g_metalib_appsvr_sdk[];
static void appsvr_haoxin_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_appsvr_haoxin_module = {
    "appsvr_haoxin_module",
    appsvr_haoxin_module_clear
};

static struct {
    const char * name; 
    int (*init)(appsvr_haoxin_module_t module);
    void (*fini)(appsvr_haoxin_module_t module);
} s_auto_reg_products[] = {
    { "attr-provider", appsvr_haoxin_attr_provider_init, appsvr_haoxin_attr_provider_fini }
    , { "backend", appsvr_haoxin_backend_init, appsvr_haoxin_backend_fini }
    , { "payment", appsvr_haoxin_payment_init, appsvr_haoxin_payment_fini }
    , { "monitor-suspend", appsvr_haoxin_suspend_monitor_init, appsvr_haoxin_suspend_monitor_fini }
};

appsvr_haoxin_module_t
appsvr_haoxin_module_create(
    gd_app_context_t app, uint8_t debug,
    app_attr_module_t app_attr_module,
    appsvr_account_module_t account_module,
    appsvr_payment_module_t payment_module,
    mem_allocrator_t alloc, const char * name, error_monitor_t em,const char * more_game_evt_name)
{
    appsvr_haoxin_module_t module;
    nm_node_t module_node;
    uint16_t component_pos;
    
    assert(app);

    if (name == NULL) name = "appsvr_haoxin_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct appsvr_haoxin_module));
    if (module_node == NULL) return NULL;

    module = (appsvr_haoxin_module_t)nm_node_data(module_node);

    module->m_app = app; 
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = debug;
    module->m_payment_module = payment_module;
    module->m_app_attr_module = app_attr_module;
    bzero(&module->m_attr_data, sizeof (module->m_attr_data));
    module->m_attr_meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_appsvr_haoxin, "appsvr_haoxin_attr");
    module->m_attr_provider = NULL;
    
    module->m_suspend_monitor = NULL;

    /*payment*/
    module->m_payment_adapter = NULL;
    module->m_payment_runing = 0;
    module->m_payment_cancel_timer = GD_TIMER_ID_INVALID;
    
    module->m_runing_id = 0;
    module->m_runing_cancel_timer = GD_TIMER_ID_INVALID;

    module->m_meta_sdk_action = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_appsvr_sdk, "appsvr_sdk_action");
    assert(module->m_meta_sdk_action);

    module->m_app_env = plugin_app_env_module_find_nc(app, NULL);
    if (module->m_app_env == NULL) {
        CPE_ERROR(em, "appsvr_haoxin_module: app env not exist!");
        nm_node_free(module_node);
        return NULL;
    }
    
    mem_buffer_init(&module->m_dump_buffer, alloc);

    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(module->m_em, "appsvr_haoxin_module_create: regist product %s fail!", s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }

            mem_buffer_clear(&module->m_dump_buffer);
            nm_node_free(module_node);
            return NULL;
        }
    }

    nm_node_set_type(module_node, &s_nm_node_type_appsvr_haoxin_module);

    return module;
}

static void appsvr_haoxin_module_clear(nm_node_t node) {
    appsvr_haoxin_module_t module = nm_node_data(node);
    uint16_t component_pos;
    
    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    mem_buffer_clear(&module->m_dump_buffer);
}

gd_app_context_t appsvr_haoxin_module_app(appsvr_haoxin_module_t module) {
    return module->m_app;
}

void appsvr_haoxin_module_free(appsvr_haoxin_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_appsvr_haoxin_module) return;
    nm_node_free(module_node);
}

appsvr_haoxin_module_t
appsvr_haoxin_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_haoxin_module) return NULL;
    return (appsvr_haoxin_module_t)nm_node_data(node);
}

appsvr_haoxin_module_t
appsvr_haoxin_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "appsvr_haoxin_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_haoxin_module) return NULL;
    return (appsvr_haoxin_module_t)nm_node_data(node);
}

const char * appsvr_haoxin_module_name(appsvr_haoxin_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

EXPORT_DIRECTIVE
int appsvr_haoxin_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    appsvr_haoxin_module_t appsvr_haoxin;
    
    appsvr_haoxin =
        appsvr_haoxin_module_create(
            app,
            cfg_get_uint8(cfg, "debug", 0),
            app_attr_module_find_nc(app, NULL),
            appsvr_account_module_find_nc(app, NULL),
            appsvr_payment_module_find_nc(app, NULL),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app),
            cfg_get_string(cfg, "more-game-notification", NULL));
    if (appsvr_haoxin == NULL) return -1;

    return 0;
}

EXPORT_DIRECTIVE
void appsvr_haoxin_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    appsvr_haoxin_module_t appsvr_haoxin;

    appsvr_haoxin = appsvr_haoxin_module_find_nc(app, gd_app_module_name(module));
    if (appsvr_haoxin) {
        appsvr_haoxin_module_free(appsvr_haoxin);
    }
}
