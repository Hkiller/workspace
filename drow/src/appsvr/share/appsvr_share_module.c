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
#include "appsvr_share_module_i.h"
#include "appsvr_share_request_i.h"
#include "appsvr_share_request_block_i.h"
#include "appsvr_share_adapter_i.h"

static void appsvr_share_module_clear(nm_node_t node);
static ptr_int_t appsvr_share_module_tick(void * ctx, ptr_int_t arg, float delta_s);

struct nm_node_type s_nm_node_type_appsvr_share_module = {
    "appsvr_share_module",
    appsvr_share_module_clear
};

/* static struct { */
/*     const char * name;  */
/*     int (*init)(appsvr_share_module_t module); */
/*     void (*fini)(appsvr_share_module_t module); */
/* } s_auto_reg_products[] = { */
/*     { "executor", appsvr_share_executor_init, appsvr_share_executor_fini }     */
/* }; */

appsvr_share_module_t
appsvr_share_module_create(
    gd_app_context_t app, uint8_t debug,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    appsvr_share_module_t module;
    nm_node_t module_node;
    
    assert(app);

    if (name == NULL) name = "appsvr_share_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct appsvr_share_module));
    if (module_node == NULL) return NULL;

    module = (appsvr_share_module_t)nm_node_data(module_node);

    module->m_app = app; 
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = debug;
    
    module->m_request_count = 0;
    module->m_request_max_id = 0;
    
    TAILQ_INIT(&module->m_adapters);
    TAILQ_INIT(&module->m_requests_to_process);
    TAILQ_INIT(&module->m_requests);
    TAILQ_INIT(&module->m_free_requests);
    TAILQ_INIT(&module->m_free_request_blocks);

    if (gd_app_tick_add(module->m_app, appsvr_share_module_tick, module, 0) != 0) {
        CPE_ERROR(module->m_em, "appsvr_share_module: add tick fail!");
        nm_node_free(module_node);
        return NULL;
    }
    
    nm_node_set_type(module_node, &s_nm_node_type_appsvr_share_module);

    return module;
}

static void appsvr_share_module_clear(nm_node_t node) {
    appsvr_share_module_t module = nm_node_data(node);

    gd_app_tick_remove(module->m_app, appsvr_share_module_tick, module);
    
    while(!TAILQ_EMPTY(&module->m_requests)) {
        appsvr_share_request_free(TAILQ_FIRST(&module->m_requests));
    }
    assert(TAILQ_EMPTY(&module->m_requests_to_process));
    
    while(!TAILQ_EMPTY(&module->m_adapters)) {
        appsvr_share_adapter_free(TAILQ_FIRST(&module->m_adapters));
    }

    while(!TAILQ_EMPTY(&module->m_free_requests)) {
        appsvr_share_request_real_free(TAILQ_FIRST(&module->m_free_requests));
    }

    while(!TAILQ_EMPTY(&module->m_free_request_blocks)) {
        appsvr_share_request_block_real_free(TAILQ_FIRST(&module->m_free_request_blocks));
    }
}

gd_app_context_t appsvr_share_module_app(appsvr_share_module_t module) {
    return module->m_app;
}

void appsvr_share_module_free(appsvr_share_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_appsvr_share_module) return;
    nm_node_free(module_node);
}

appsvr_share_module_t
appsvr_share_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_share_module) return NULL;
    return (appsvr_share_module_t)nm_node_data(node);
}

appsvr_share_module_t
appsvr_share_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "appsvr_share_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_share_module) return NULL;
    return (appsvr_share_module_t)nm_node_data(node);
}

const char * appsvr_share_module_name(appsvr_share_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

static ptr_int_t appsvr_share_module_tick(void * ctx, ptr_int_t arg, float delta_s) {
    appsvr_share_module_t module = ctx;

    appsvr_share_request_tick(module);

    return 0;
}

EXPORT_DIRECTIVE
int appsvr_share_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    appsvr_share_module_t share_module;
    
    share_module =
        appsvr_share_module_create(
            app,
            cfg_get_uint8(cfg, "debug", 0),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (share_module == NULL) return -1;

    return 0;
}

EXPORT_DIRECTIVE
void appsvr_share_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    appsvr_share_module_t share_module;

    share_module = appsvr_share_module_find_nc(app, gd_app_module_name(module));
    if (share_module) {
        appsvr_share_module_free(share_module);
    }
}
