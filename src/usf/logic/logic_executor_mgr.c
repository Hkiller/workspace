#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "usf/logic/logic_executor.h"
#include "usf/logic/logic_executor_ref.h"
#include "usf/logic/logic_executor_mgr.h"
#include "usf/logic/logic_executor_build.h"
#include "usf/logic/logic_context.h"
#include "logic_internal_ops.h"

static void logic_executor_mgr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_logic_executor_mgr = {
    "usf_logic_executor_mgr",
    logic_executor_mgr_clear
};

logic_executor_mgr_t
logic_executor_mgr_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t  em)
{
    logic_executor_mgr_t mgr;
    nm_node_t mgr_node;

    mgr_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct logic_executor_mgr));
    if (mgr_node == NULL) return NULL;

    mgr = (logic_executor_mgr_t)nm_node_data(mgr_node);
    mgr->m_alloc = alloc;
    mgr->m_app = app;
    mgr->m_em = em;
    mgr->m_debug = 0;

    if (cpe_hash_table_init(
            &mgr->m_executor_refs,
            alloc,
            (cpe_hash_fun_t) logic_executor_ref_hash,
            (cpe_hash_eq_t) logic_executor_ref_cmp,
            CPE_HASH_OBJ2ENTRY(logic_executor_ref, m_hh),
            -1) != 0)
    {
        nm_node_free(mgr_node);
        return NULL;
    }

    nm_node_set_type(mgr_node, &s_nm_node_type_logic_executor_mgr);

    return mgr;
}

static void logic_executor_mgr_clear(nm_node_t node) {
    logic_executor_mgr_t mgr;
    mgr = (logic_executor_mgr_t)nm_node_data(node);
    logic_executor_ref_free_all(mgr);
    cpe_hash_table_fini(&mgr->m_executor_refs);
}

void logic_executor_mgr_free(logic_executor_mgr_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_logic_executor_mgr) return;
    nm_node_free(mgr_node);
}

logic_executor_mgr_t
logic_executor_mgr_find(
    gd_app_context_t app,
    cpe_hash_string_t name)
{
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_logic_executor_mgr) return NULL;
    return (logic_executor_mgr_t)nm_node_data(node);
}

logic_executor_mgr_t
logic_executor_mgr_find_nc(
    gd_app_context_t app,
    const char * name)
{
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_logic_executor_mgr) return NULL;
    return (logic_executor_mgr_t)nm_node_data(node);
}

gd_app_context_t logic_executor_mgr_app(logic_executor_mgr_t mgr) {
    return mgr->m_app;
}

const char * logic_executor_mgr_name(logic_executor_mgr_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
logic_executor_mgr_name_hs(logic_executor_mgr_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

logic_executor_ref_t
logic_executor_mgr_import(
    logic_executor_mgr_t mgr,
    const char * name,
    logic_manage_t logic_mgr,
    logic_executor_type_group_t type_group,
    cfg_t cfg)
{
    logic_executor_t executor;
    logic_executor_ref_t ref;

    assert(mgr);

    executor = logic_executor_build(
        logic_mgr,
        cfg,
        type_group,
        mgr->m_em);
    if (executor == NULL) return NULL;

    ref = logic_executor_ref_create(mgr, name, executor);
    if (ref == NULL) {
        logic_executor_free(executor);
        return NULL;
    }

    return ref;
}

EXPORT_DIRECTIVE
int logic_executor_mgr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    logic_executor_mgr_t logic_executor_mgr;

    logic_executor_mgr = logic_executor_mgr_create(app, gd_app_module_name(module), gd_app_alloc(app), gd_app_em(app));
    if (logic_executor_mgr == NULL) return -1;

    logic_executor_mgr->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (logic_executor_mgr->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            logic_executor_mgr_name(logic_executor_mgr));
    }

    return 0;
}

EXPORT_DIRECTIVE
void logic_executor_mgr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    logic_executor_mgr_t logic_executor_mgr;

    logic_executor_mgr = logic_executor_mgr_find_nc(app, gd_app_module_name(module));
    if (logic_executor_mgr) {
        logic_executor_mgr_free(logic_executor_mgr);
    }
}

