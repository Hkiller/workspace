#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "plugin/ui/plugin_ui_module.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_state.h"
#include "plugin/ui/plugin_ui_phase.h"
#include "plugin/ui/plugin_ui_state_node.h"
#include "plugin/ui/plugin_ui_phase_node.h"
#include "appsvr_ad_module_i.h"
#include "appsvr_ad_adapter_i.h"
#include "appsvr_ad_request_i.h"
#include "appsvr_ad_action_i.h"
#include "appsvr_ad_ui_state_monitor_i.h"

static void appsvr_ad_module_clear(nm_node_t node);
static ptr_int_t appsvr_ad_module_tick(void * ctx, ptr_int_t arg, float delta_s);

struct nm_node_type s_nm_node_type_appsvr_ad_module = {
    "appsvr_ad_module",
    appsvr_ad_module_clear
};

/* static struct { */
/*     const char * name;  */
/*     int (*init)(appsvr_ad_module_t module); */
/*     void (*fini)(appsvr_ad_module_t module); */
/* } s_auto_reg_products[] = { */
/*     { "executor", appsvr_ad_executor_init, appsvr_ad_executor_fini }     */
/* }; */

appsvr_ad_module_t
appsvr_ad_module_create(
    gd_app_context_t app, uint8_t debug,
    plugin_ui_module_t ui_module, 
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    appsvr_ad_module_t module;
    nm_node_t module_node;
    
    assert(app);

    if (name == NULL) name = "appsvr_ad_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct appsvr_ad_module));
    if (module_node == NULL) return NULL;

    module = (appsvr_ad_module_t)nm_node_data(module_node);

    module->m_app = app; 
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = debug;
    module->m_adapter_count = 0;
    module->m_request_count = 0;
    module->m_request_max_id = 0;
    module->m_ui_module = ui_module;
    module->m_ui_cur_state = NULL;
    module->m_ui_cur_state_processed = 0;
    
    if (gd_app_tick_add(module->m_app, appsvr_ad_module_tick, module, 0) != 0) {
        CPE_ERROR(module->m_em, "appsvr_ad_module: add tick fail!");
        nm_node_free(module_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &module->m_actions,
            alloc,
            (cpe_hash_fun_t) appsvr_ad_action_hash,
            (cpe_hash_eq_t) appsvr_ad_action_eq,
            CPE_HASH_OBJ2ENTRY(appsvr_ad_action, m_hh),
            -1) != 0)
    {
        gd_app_tick_remove(module->m_app, appsvr_ad_module_tick, module);
        nm_node_free(module_node);
        return NULL;
    }
    
    TAILQ_INIT(&module->m_adapters);
    TAILQ_INIT(&module->m_requests);
    TAILQ_INIT(&module->m_free_requests);
    TAILQ_INIT(&module->m_ui_state_monitors);
    
    nm_node_set_type(module_node, &s_nm_node_type_appsvr_ad_module);

    return module;
}

static void appsvr_ad_module_clear(nm_node_t node) {
    appsvr_ad_module_t module = nm_node_data(node);

    gd_app_tick_remove(module->m_app, appsvr_ad_module_tick, module);

    while(!TAILQ_EMPTY(&module->m_ui_state_monitors)) {
        appsvr_ad_ui_state_monitor_free(TAILQ_FIRST(&module->m_ui_state_monitors));
    }

    while(!TAILQ_EMPTY(&module->m_requests)) {
        appsvr_ad_request_free(TAILQ_FIRST(&module->m_requests));
    }
    
    while(!TAILQ_EMPTY(&module->m_adapters)) {
        appsvr_ad_adapter_free(TAILQ_FIRST(&module->m_adapters));
    }

    assert(cpe_hash_table_count(&module->m_actions) == 0);
    cpe_hash_table_fini(&module->m_actions);

    while(!TAILQ_EMPTY(&module->m_free_requests)) {
        appsvr_ad_request_real_free(TAILQ_FIRST(&module->m_free_requests));
    }
}

gd_app_context_t appsvr_ad_module_app(appsvr_ad_module_t module) {
    return module->m_app;
}

void appsvr_ad_module_free(appsvr_ad_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_appsvr_ad_module) return;
    nm_node_free(module_node);
}

appsvr_ad_module_t
appsvr_ad_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_ad_module) return NULL;
    return (appsvr_ad_module_t)nm_node_data(node);
}

appsvr_ad_module_t
appsvr_ad_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "appsvr_ad_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_ad_module) return NULL;
    return (appsvr_ad_module_t)nm_node_data(node);
}

const char * appsvr_ad_module_name(appsvr_ad_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

int appsvr_ad_module_start(
    appsvr_ad_module_t module, const char * action_name, uint32_t * r,
    void * ctx, appsvr_ad_resonse_fun_t response_fun, void * arg, void (*arg_free_fun)(void * ctx))
{
    appsvr_ad_action_t action;
    appsvr_ad_request_t request;

    action = appsvr_ad_action_find(module, action_name);
    if (action == NULL) {
        if (module->m_debug) {
            CPE_INFO(module->m_em, "appsvr_ad_module_start: action %s: action not exist!", action_name);
        }
    }

    request = appsvr_ad_request_create(module, action, ctx, response_fun, arg, arg_free_fun);
    if (request == NULL) {
        CPE_ERROR(module->m_em, "appsvr_ad_module_start: action %s: request create fail!", action_name);
        return -1;
    }

    if (r) *r = request->m_id;
    
    return 0;
}

int appsvr_ad_module_remove_by_id(appsvr_ad_module_t module, uint32_t request_id) {
    appsvr_ad_request_t request;

    request = appsvr_ad_request_find_by_id(module, request_id);
    if (request == NULL) return -1;

    appsvr_ad_request_free(request);

    return 0;
}

uint32_t appsvr_ad_module_remove_by_ctx(appsvr_ad_module_t module, void * ctx) {
    appsvr_ad_request_t request, next_request;
    uint32_t count = 0;

    for(request = TAILQ_FIRST(&module->m_requests); request; request = next_request) {
        next_request = TAILQ_NEXT(request, m_next_for_module);

        if (request->m_ctx == ctx) {
            appsvr_ad_request_free(request);
            count++;
        }
    }

    return count;
}

/* static void appsvr_ad_state_notify_state(appsvr_ad_module_t module, plugin_ui_state_t state, uint8_t is_open) { */
/*     char state_name[64]; */
/*     snprintf( */
/*         state_name, sizeof(state_name), "%s.%s", */
/*         plugin_ui_phase_name(plugin_ui_state_phase(state)), */
/*         plugin_ui_state_name(state)); */
    
/*     if (is_open) { */
/*         //appsvr_ad_on_state_begin(module, state_name); */
/*     } */
/*     else { */
/*         //appsvr_ad_on_state_end(module, state_name); */
/*     } */
/* } */

static ptr_int_t appsvr_ad_module_tick(void * ctx, ptr_int_t arg, float delta_s) {
    appsvr_ad_module_t module = ctx;
    appsvr_ad_request_t request, next_request;

    for(request = TAILQ_FIRST(&module->m_requests); request; request = next_request) {
        next_request = TAILQ_NEXT(request, m_next_for_module);

        if (!request->m_in_process) {
            if (request->m_response_fun) {
                request->m_response_fun(request->m_ctx, request->m_arg, request->m_id, request->m_result);
            }

            appsvr_ad_request_free(request);
        }
    }
    
    if (!TAILQ_EMPTY(&module->m_ui_state_monitors) && module->m_ui_module) {
        plugin_ui_state_node_t cur_state_node = NULL;
        plugin_ui_state_t cur_state = NULL;
        plugin_ui_env_t cur_env;

        cur_env = plugin_ui_env_first(module->m_ui_module);
        if (cur_env) {
            plugin_ui_phase_node_t cur_phase_node = plugin_ui_phase_node_current(cur_env);
            if (cur_phase_node) {
                cur_state_node = plugin_ui_state_node_current(cur_phase_node);
                if (cur_state_node) {
                    cur_state = plugin_ui_state_node_process_state(cur_state_node);
                }
            }
        }

        if (cur_state != module->m_ui_cur_state) {
            cur_state = module->m_ui_cur_state;
            module->m_ui_cur_state_processed = 0;
        }

        if (!module->m_ui_cur_state_processed) {
            if (plugin_ui_state_node_state(cur_state_node) == plugin_ui_state_node_state_processing) {
                appsvr_ad_ui_state_monitor_t monitor;
                
                module->m_ui_cur_state_processed = 1;

                monitor = appsvr_ad_ui_state_monitor_find_by_state_name(module, plugin_ui_state_name(cur_state));
                if (monitor) {
                    appsvr_ad_module_start(module, monitor->m_action_name, NULL, NULL, NULL, NULL, NULL);
                }
            }
        }
    }
    
    return 0;
}

EXPORT_DIRECTIVE
int appsvr_ad_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    appsvr_ad_module_t ad;
    
    ad =
        appsvr_ad_module_create(
            app,
            cfg_get_uint8(cfg, "debug", 0),
            plugin_ui_module_find_nc(app, NULL),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (ad == NULL) return -1;

    return 0;
}

EXPORT_DIRECTIVE
void appsvr_ad_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    appsvr_ad_module_t appsvr_ad_module;

    appsvr_ad_module = appsvr_ad_module_find_nc(app, gd_app_module_name(module));
    if (appsvr_ad_module) {
        appsvr_ad_module_free(appsvr_ad_module);
    }
}
