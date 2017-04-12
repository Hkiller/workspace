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
#include "plugin/ui/plugin_ui_module.h"
#include "appsvr_umeng_module_i.h"
#include "appsvr_umeng_executor.h"
#include "appsvr_umeng_pay_chanel.h"
#include "appsvr_umeng_click_info.h"

static void appsvr_umeng_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_appsvr_umeng_module = {
    "appsvr_umeng_module",
    appsvr_umeng_module_clear
};

static struct {
    const char * name; 
    int (*init)(appsvr_umeng_module_t module);
    void (*fini)(appsvr_umeng_module_t module);
} s_auto_reg_products[] = {
    { "backend", appsvr_umeng_backend_init, appsvr_umeng_backend_fini }
    , { "suspend-monitor", appsvr_umeng_suspend_monitor_init, appsvr_umeng_suspend_monitor_fini }
    , { "click-monitor", appsvr_umeng_click_monitor_init, appsvr_umeng_click_monitor_fini }
    , { "page-monitor", appsvr_umeng_page_monitor_init, appsvr_umeng_page_monitor_fini }
};

appsvr_umeng_module_t
appsvr_umeng_module_create(
    gd_app_context_t app, plugin_ui_module_t ui_module, const char * app_key, const char * chanel,
    uint8_t debug,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    appsvr_umeng_module_t module;
    nm_node_t module_node;
    uint16_t component_pos;
    
    assert(app);

    if (name == NULL) name = "appsvr_umeng_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct appsvr_umeng_module));
    if (module_node == NULL) return NULL;

    module = (appsvr_umeng_module_t)nm_node_data(module_node);

    module->m_app = app; 
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = debug;
    module->m_app_key = cpe_str_mem_dup(module->m_alloc, app_key);
    module->m_chanel = cpe_str_mem_dup(module->m_alloc, chanel);
    
    module->m_ui_module = ui_module;
    module->m_cur_state = NULL;
    module->m_click_monitor = NULL;

    module->m_app_env = plugin_app_env_module_find_nc(app, NULL);
    if (module->m_app_env == NULL) {
        CPE_ERROR(em, "appsvr_umeng_module: app env not exist!");
        nm_node_free(module_node);
        return NULL;
    }

    module->m_computer = xcomputer_create(alloc, em);
    if (module->m_computer == NULL) {
        CPE_ERROR(em, "appsvr_umeng_module: create computer fail!");
        nm_node_free(module_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &module->m_click_infos,
            module->m_alloc,
            (cpe_hash_fun_t) appsvr_umeng_click_info_hash,
            (cpe_hash_eq_t) appsvr_umeng_click_info_eq,
            CPE_HASH_OBJ2ENTRY(appsvr_umeng_click_info, m_hh),
            -1) != 0)
    {
        xcomputer_free(module->m_computer);
        mem_free(alloc, module);
        return NULL;
    }
    
    mem_buffer_init(&module->m_dump_buffer, alloc);

    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(module->m_em, "appsvr_umeng_module_create: regist product %s fail!", s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }

            mem_buffer_clear(&module->m_dump_buffer);
            cpe_hash_table_fini(&module->m_click_infos);
            xcomputer_free(module->m_computer);
            nm_node_free(module_node);
            return NULL;
        }
    }

    TAILQ_INIT(&module->m_executors);
    TAILQ_INIT(&module->m_pay_chanels);
    
    nm_node_set_type(module_node, &s_nm_node_type_appsvr_umeng_module);

    return module;
}

static void appsvr_umeng_module_clear(nm_node_t node) {
    appsvr_umeng_module_t module = nm_node_data(node);
    uint16_t component_pos;

    while(!TAILQ_EMPTY(&module->m_executors)) {
        appsvr_umeng_executor_free(TAILQ_FIRST(&module->m_executors));
    }

    while(!TAILQ_EMPTY(&module->m_pay_chanels)) {
        appsvr_umeng_pay_chanel_free(TAILQ_FIRST(&module->m_pay_chanels));
    }

    appsvr_umeng_click_info_free_all(module);
    cpe_hash_table_fini(&module->m_click_infos);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    assert(module->m_computer);
    xcomputer_free(module->m_computer);
    module->m_computer = NULL;

    mem_free(module->m_alloc, module->m_app_key);
    module->m_app_key = NULL;

    mem_free(module->m_alloc, module->m_chanel);
    module->m_chanel = NULL;

    mem_buffer_clear(&module->m_dump_buffer);
}

gd_app_context_t appsvr_umeng_module_app(appsvr_umeng_module_t module) {
    return module->m_app;
}

void appsvr_umeng_module_free(appsvr_umeng_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_appsvr_umeng_module) return;
    nm_node_free(module_node);
}

appsvr_umeng_module_t
appsvr_umeng_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_umeng_module) return NULL;
    return (appsvr_umeng_module_t)nm_node_data(node);
}

appsvr_umeng_module_t
appsvr_umeng_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "appsvr_umeng_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_umeng_module) return NULL;
    return (appsvr_umeng_module_t)nm_node_data(node);
}

const char * appsvr_umeng_module_name(appsvr_umeng_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

EXPORT_DIRECTIVE
int appsvr_umeng_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    appsvr_umeng_module_t appsvr_umeng;
    const char * app_key;
    const char * chanel;
    
#if defined ANDROID    
    app_key = cfg_get_string(cfg, "app-key.android", NULL);
#elif defined _APPLE
    app_key = cfg_get_string(cfg, "app-key.ios", NULL);
#else
    app_key = "";
#endif

    if (app_key == NULL) {
        APP_CTX_ERROR(app, "appsvr_umeng_module: create: app-key not configured!");
        return -1;
    }

    chanel = cfg_get_string(gd_app_cfg(app), "args.chanel", NULL);
    if (chanel == NULL) {
        APP_CTX_ERROR(app, "appsvr_umeng_module: create: app-key not configured!");
        return -1;
    }

    appsvr_umeng =
        appsvr_umeng_module_create(
            app,
            plugin_ui_module_find_nc(app, NULL),
            app_key,
            chanel,
            cfg_get_uint8(cfg, "debug", 0),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (appsvr_umeng == NULL) return -1;

    if (appsvr_umeng_load_executors(appsvr_umeng, cfg) != 0) {
        APP_CTX_ERROR(app, "appsvr_umeng_module: create: load executors fail!");
        appsvr_umeng_module_free(appsvr_umeng);
        return -1;
    }

    if (appsvr_umeng_load_pay_chanels(appsvr_umeng, cfg) != 0) {
        APP_CTX_ERROR(app, "appsvr_umeng_module: create: load pay chanels fail!");
        appsvr_umeng_module_free(appsvr_umeng);
        return -1;
    }
    
    if (appsvr_umeng_load_click_infos(appsvr_umeng, cfg) != 0) {
        APP_CTX_ERROR(app, "appsvr_umeng_module: create: load click infos fail!");
        appsvr_umeng_module_free(appsvr_umeng);
        return -1;
    }
    
    return 0;
}

EXPORT_DIRECTIVE
void appsvr_umeng_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    appsvr_umeng_module_t appsvr_umeng;

    appsvr_umeng = appsvr_umeng_module_find_nc(app, gd_app_module_name(module));
    if (appsvr_umeng) {
        appsvr_umeng_module_free(appsvr_umeng);
    }
}
