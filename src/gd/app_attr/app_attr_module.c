#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "app_attr_module_i.h"
#include "app_attr_request_i.h"
#include "app_attr_formula_i.h"
#include "app_attr_provider_i.h"
#include "app_attr_synchronizer_i.h"
#include "app_attr_attr_i.h"
#include "app_attr_attr_binding_i.h"

static void app_attr_module_clear(nm_node_t node);
static ptr_int_t app_attr_module_tick(void * ctx, ptr_int_t arg, float delta_s);

struct nm_node_type s_nm_node_type_app_attr_module = {
    "app_attr_module",
    app_attr_module_clear
};

app_attr_module_t
app_attr_module_create(
    gd_app_context_t app, uint8_t debug,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    app_attr_module_t module;
    nm_node_t module_node;
    
    assert(app);

    if (name == NULL) name = "app_attr_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct app_attr_module));
    if (module_node == NULL) return NULL;

    module = (app_attr_module_t)nm_node_data(module_node);

    module->m_app = app; 
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = debug;
    module->m_computer = xcomputer_create(alloc, em);
    if (module->m_computer == NULL) {
        CPE_ERROR(module->m_em, "app_attr_module: create computer fail!");
        nm_node_free(module_node);
        return NULL;
    }
    
    module->m_request_count = 0;
    module->m_request_max_id = 0;

    TAILQ_INIT(&module->m_providers);
    TAILQ_INIT(&module->m_requests_to_process);
    TAILQ_INIT(&module->m_requests);
    TAILQ_INIT(&module->m_synchronizer_to_process);
    TAILQ_INIT(&module->m_free_requests);
    TAILQ_INIT(&module->m_free_formulas);
    TAILQ_INIT(&module->m_free_attr_bindings);

    if (gd_app_tick_add(module->m_app, app_attr_module_tick, module, 0) != 0) {
        CPE_ERROR(module->m_em, "app_attr_module: add tick fail!");
        xcomputer_free(module->m_computer);
        nm_node_free(module_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &module->m_attrs,
            alloc,
            (cpe_hash_fun_t) app_attr_attr_hash,
            (cpe_hash_eq_t) app_attr_attr_eq,
            CPE_HASH_OBJ2ENTRY(app_attr_attr, m_hh),
            -1) != 0)
    {
        gd_app_tick_remove(module->m_app, app_attr_module_tick, module);
        xcomputer_free(module->m_computer);
        nm_node_free(module_node);
        return NULL;
    }
    
    nm_node_set_type(module_node, &s_nm_node_type_app_attr_module);

    return module;
}

static void app_attr_module_clear(nm_node_t node) {
    app_attr_module_t module = nm_node_data(node);

    gd_app_tick_remove(module->m_app, app_attr_module_tick, module);
    
    while(!TAILQ_EMPTY(&module->m_requests)) {
        app_attr_request_free(TAILQ_FIRST(&module->m_requests));
    }
    assert(TAILQ_EMPTY(&module->m_requests_to_process));
    
    while(!TAILQ_EMPTY(&module->m_providers)) {
        app_attr_provider_free(TAILQ_FIRST(&module->m_providers));
    }

    assert(cpe_hash_table_count(&module->m_attrs) == 0);
    cpe_hash_table_fini(&module->m_attrs);

    xcomputer_free(module->m_computer);
    
    while(!TAILQ_EMPTY(&module->m_free_requests)) {
        app_attr_request_real_free(TAILQ_FIRST(&module->m_free_requests));
    }

    while(!TAILQ_EMPTY(&module->m_free_formulas)) {
        app_attr_formula_real_free(TAILQ_FIRST(&module->m_free_formulas));
    }
    
    while(!TAILQ_EMPTY(&module->m_free_attr_bindings)) {
        app_attr_attr_binding_real_free(TAILQ_FIRST(&module->m_free_attr_bindings));
    }
}

gd_app_context_t app_attr_module_app(app_attr_module_t module) {
    return module->m_app;
}

void app_attr_module_free(app_attr_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_app_attr_module) return;
    nm_node_free(module_node);
}

app_attr_module_t
app_attr_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_app_attr_module) return NULL;
    return (app_attr_module_t)nm_node_data(node);
}

app_attr_module_t
app_attr_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "app_attr_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_app_attr_module) return NULL;
    return (app_attr_module_t)nm_node_data(node);
}

const char * app_attr_module_name(app_attr_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

static ptr_int_t app_attr_module_tick(void * ctx, ptr_int_t arg, float delta_s) {
    app_attr_module_t module = ctx;

    app_attr_request_tick(module);
    app_attr_synchronizer_tick(module);

    return 0;
}

EXPORT_DIRECTIVE
int app_attr_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    app_attr_module_t share_module;
    
    share_module =
        app_attr_module_create(
            app,
            cfg_get_uint8(cfg, "debug", 0),
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (share_module == NULL) return -1;

    return 0;
}

EXPORT_DIRECTIVE
void app_attr_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    app_attr_module_t share_module;

    share_module = app_attr_module_find_nc(app, gd_app_module_name(module));
    if (share_module) {
        app_attr_module_free(share_module);
    }
}
