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
#include "appsvr_payment_module_i.h"
#include "appsvr_payment_executor.h"
#include "appsvr_payment_adapter_i.h"
#include "appsvr_payment_product_i.h"
#include "appsvr_payment_product_request_i.h"

extern char g_metalib_appsvr_payment[];
static void appsvr_payment_module_clear(nm_node_t node);
static ptr_int_t appsvr_payment_module_tick(void * ctx, ptr_int_t arg, float delta_s);

struct nm_node_type s_nm_node_type_appsvr_payment_module = {
    "appsvr_payment_module",
    appsvr_payment_module_clear
};

static struct {
    const char * name; 
    int (*init)(appsvr_payment_module_t module);
    void (*fini)(appsvr_payment_module_t module);
} s_auto_reg_products[] = {
    { "executor", appsvr_payment_executor_init, appsvr_payment_executor_fini }    
};

appsvr_payment_module_t
appsvr_payment_module_create(
    gd_app_context_t app, uint8_t debug,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    appsvr_payment_module_t module;
    nm_node_t module_node;
    uint16_t component_pos;
    
    assert(app);

    if (name == NULL) name = "appsvr_payment_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct appsvr_payment_module));
    if (module_node == NULL) return NULL;

    module = (appsvr_payment_module_t)nm_node_data(module_node);

    module->m_app = app; 
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = debug;
    module->m_adapter_count = 0;
    
    module->m_runing_adapter = NULL;
    module->m_runing_id = 0;

    module->m_meta_req_pay = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_appsvr_payment, "appsvr_payment_buy");
    assert(module->m_meta_req_pay);
    module->m_meta_res_pay = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_appsvr_payment, "appsvr_payment_result");
    assert(module->m_meta_res_pay);

    module->m_meta_req_query_services = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_appsvr_payment, "appsvr_payment_query_services");
    assert(module->m_meta_req_query_services);
    module->m_meta_res_query_services = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_appsvr_payment, "appsvr_payment_service_list");
    assert(module->m_meta_res_query_services);

    TAILQ_INIT(&module->m_products);
    TAILQ_INIT(&module->m_product_requests);
    
    module->m_app_env = plugin_app_env_module_find_nc(app, NULL);
    if (module->m_app_env == NULL) {
        CPE_ERROR(em, "appsvr_payment_module: app env not exist!");
        nm_node_free(module_node);
        return NULL;
    }

    if (gd_app_tick_add(module->m_app, appsvr_payment_module_tick, module, 0) != 0) {
        CPE_ERROR(module->m_em, "appsvr_payment_module: add tick fail!");
        nm_node_free(module_node);
        return NULL;
    }
    
    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(module->m_em, "appsvr_payment_module_create: regist product %s fail!", s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }
            gd_app_tick_remove(module->m_app, appsvr_payment_module_tick, module);
            nm_node_free(module_node);
            return NULL;
        }
    }

    TAILQ_INIT(&module->m_adapters);
    mem_buffer_init(&module->m_dump_buffer, alloc);

    nm_node_set_type(module_node, &s_nm_node_type_appsvr_payment_module);

    return module;
}

static void appsvr_payment_module_clear(nm_node_t node) {
    appsvr_payment_module_t module = nm_node_data(node);
    uint16_t component_pos;

    gd_app_tick_remove(module->m_app, appsvr_payment_module_tick, module);
    
    while(!TAILQ_EMPTY(&module->m_product_requests)) {
        appsvr_payment_product_request_free(TAILQ_FIRST(&module->m_product_requests));
    }
    
    while(!TAILQ_EMPTY(&module->m_adapters)) {
        appsvr_payment_adapter_free(TAILQ_FIRST(&module->m_adapters));
    }
    assert(TAILQ_EMPTY(&module->m_products));

    assert(module->m_runing_adapter == NULL);
    assert(module->m_runing_id == 0);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    mem_buffer_clear(&module->m_dump_buffer);
}

gd_app_context_t appsvr_payment_module_app(appsvr_payment_module_t module) {
    return module->m_app;
}

void appsvr_payment_module_free(appsvr_payment_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_appsvr_payment_module) return;
    nm_node_free(module_node);
}

appsvr_payment_module_t
appsvr_payment_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_payment_module) return NULL;
    return (appsvr_payment_module_t)nm_node_data(node);
}

appsvr_payment_module_t
appsvr_payment_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "appsvr_payment_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_payment_module) return NULL;
    return (appsvr_payment_module_t)nm_node_data(node);
}

const char * appsvr_payment_module_name(appsvr_payment_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

static ptr_int_t appsvr_payment_module_tick(void * ctx, ptr_int_t arg, float delta_s) {
    appsvr_payment_module_t module = ctx;
    appsvr_payment_product_request_t product_request, next_product_request;

    for(product_request = TAILQ_FIRST(&module->m_product_requests); product_request; product_request = next_product_request) {
        next_product_request = TAILQ_NEXT(product_request, m_next_for_module);

        if (!TAILQ_EMPTY(&product_request->m_runings)) continue;
        
        if (product_request->m_response_fun) {
            product_request->m_response_fun(product_request->m_ctx, module, product_request->m_arg);
        }

        appsvr_payment_product_request_free(product_request);
    }
    
    return 0;
}

EXPORT_DIRECTIVE
int appsvr_payment_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    appsvr_payment_module_t payment;
    struct cfg_it adapters_it;
    cfg_t adapter_cfg;
    
    payment =
        appsvr_payment_module_create(
            app,
            cfg_get_uint8(cfg, "debug", 0),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (payment == NULL) return -1;

    cfg_it_init(&adapters_it, cfg_find_cfg(gd_app_cfg(app), "payment"));
    while((adapter_cfg = cfg_it_next(&adapters_it))) {
        char type_buf[64];
        const char * service_name;
        appsvr_payment_adapter_creation_fun_t creator;

        service_name = cfg_name(adapter_cfg);
        if (service_name == NULL) {
            APP_CTX_ERROR(app, "payment: load: read service type error");
            appsvr_payment_module_free(payment);
            return -1;
        }

        snprintf(type_buf, sizeof(type_buf), "appsvr_payment_adapter_%s_create", service_name);
        creator = gd_app_lib_sym(NULL, type_buf, payment->m_em);
        if (creator == NULL) {
            APP_CTX_INFO(app, "payment: load: service %s not exist, ignore", service_name);
            continue;
        }

        if (creator(payment, adapter_cfg, payment->m_alloc, payment->m_em) == NULL) {
            APP_CTX_ERROR(app, "payment: load: service %s create fail!", service_name);
            appsvr_payment_module_free(payment);
            return -1;
        }

        APP_CTX_INFO(app, "payment: load: service %s load success", service_name);
    }
    
    return 0;
}

EXPORT_DIRECTIVE
void appsvr_payment_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    appsvr_payment_module_t appsvr_payment_module;

    appsvr_payment_module = appsvr_payment_module_find_nc(app, gd_app_module_name(module));
    if (appsvr_payment_module) {
        appsvr_payment_module_free(appsvr_payment_module);
    }
}
