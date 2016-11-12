#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/timer/timer_manage.h"
#include "usf/logic/logic_manage.h"
#include "usf/logic/logic_context.h"
#include "logic_internal_ops.h"

static void logic_manage_clear(nm_node_t node);
static ptr_int_t logic_manage_tick(void * ctx, ptr_int_t arg, float delta);

static cpe_hash_string_buf s_logic_manage_default_name = CPE_HS_BUF_MAKE("logic_manage");

struct nm_node_type s_nm_node_type_logic_manage = {
    "usf_logic_manage",
    logic_manage_clear
};


logic_manage_t
logic_manage_create(
    gd_app_context_t app,
    gd_timer_mgr_t timer_mgr,
    const char * name,
    mem_allocrator_t alloc)
{
    logic_manage_t mgr;
    nm_node_t mgr_node;

    if (name == 0) name = cpe_hs_data((cpe_hash_string_t)&s_logic_manage_default_name);

    mgr_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct logic_manage));
    if (mgr_node == NULL) return NULL;

    mgr = (logic_manage_t)nm_node_data(mgr_node);
    mgr->m_alloc = alloc;
    mgr->m_app = app;
    mgr->m_timer_mgr = timer_mgr;
    mgr->m_require_timout_ms = 0;
    mgr->m_context_timout_ms = 0;
    mgr->m_context_id = 1;
    mgr->m_require_id = 1;
    mgr->m_debug = 0;
    mgr->m_waiting_count = 0;
    TAILQ_INIT(&mgr->m_waiting_contexts);
    mgr->m_pending_count = 0;
    TAILQ_INIT(&mgr->m_pending_contexts);

    if (cpe_hash_table_init(
            &mgr->m_requires,
            alloc,
            (cpe_hash_fun_t) logic_require_hash,
            (cpe_hash_eq_t) logic_require_cmp,
            CPE_HASH_OBJ2ENTRY(logic_require, m_hh),
            -1) != 0)
    {
        nm_node_free(mgr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &mgr->m_datas,
            alloc,
            (cpe_hash_fun_t) logic_data_hash,
            (cpe_hash_eq_t) logic_data_cmp,
            CPE_HASH_OBJ2ENTRY(logic_data, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&mgr->m_requires);
        nm_node_free(mgr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &mgr->m_contexts,
            alloc,
            (cpe_hash_fun_t) logic_context_hash,
            (cpe_hash_eq_t) logic_context_cmp,
            CPE_HASH_OBJ2ENTRY(logic_context, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&mgr->m_requires);
        cpe_hash_table_fini(&mgr->m_datas);
        nm_node_free(mgr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &mgr->m_queues,
            alloc,
            (cpe_hash_fun_t) logic_queue_hash,
            (cpe_hash_eq_t) logic_queue_cmp,
            CPE_HASH_OBJ2ENTRY(logic_queue, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&mgr->m_requires);
        cpe_hash_table_fini(&mgr->m_datas);
        cpe_hash_table_fini(&mgr->m_contexts);
        nm_node_free(mgr_node);
        return NULL;
    }

    if (gd_app_tick_add(app, logic_manage_tick, mgr, (ptr_int_t)500) != 0) {
        cpe_hash_table_fini(&mgr->m_requires);
        cpe_hash_table_fini(&mgr->m_datas);
        cpe_hash_table_fini(&mgr->m_contexts);
        cpe_hash_table_fini(&mgr->m_queues);
        nm_node_free(mgr_node);
        return NULL;
    }

    nm_node_set_type(mgr_node, &s_nm_node_type_logic_manage);

    return mgr;
}

static void logic_manage_clear(nm_node_t node) {
    logic_manage_t mgr;
    mgr = (logic_manage_t)nm_node_data(node);

    gd_app_tick_remove(mgr->m_app, logic_manage_tick, mgr);

    logic_context_free_all(mgr);
    logic_require_free_all(mgr);
    logic_data_free_all(mgr);
    logic_queue_free_all(mgr);

    assert(TAILQ_EMPTY(&mgr->m_waiting_contexts));
    assert(TAILQ_EMPTY(&mgr->m_pending_contexts));

    assert(TAILQ_EMPTY(&mgr->m_waiting_contexts));
    assert(TAILQ_EMPTY(&mgr->m_pending_contexts));

    cpe_hash_table_fini(&mgr->m_contexts);
    cpe_hash_table_fini(&mgr->m_requires);
    cpe_hash_table_fini(&mgr->m_datas);
    cpe_hash_table_fini(&mgr->m_queues);
}

void logic_manage_free(logic_manage_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_logic_manage) return;
    nm_node_free(mgr_node);
}

logic_manage_t
logic_manage_find(
    gd_app_context_t app,
    cpe_hash_string_t name)
{
    nm_node_t node;
    if (name == NULL) name = (cpe_hash_string_t)&s_logic_manage_default_name;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_logic_manage) return NULL;
    return (logic_manage_t)nm_node_data(node);
}

logic_manage_t
logic_manage_find_nc(
    gd_app_context_t app,
    const char * name)
{
    nm_node_t node;

    if (name == NULL) return logic_manage_default(app);

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_logic_manage) return NULL;
    return (logic_manage_t)nm_node_data(node);
}

logic_manage_t
logic_manage_default(
    gd_app_context_t app)
{
    return logic_manage_find(app, (cpe_hash_string_t)&s_logic_manage_default_name);
}

gd_app_context_t logic_manage_app(logic_manage_t mgr) {
    return mgr->m_app;
}

error_monitor_t logic_manage_em(logic_manage_t mgr) {
    return gd_app_em(mgr->m_app);
}

const char * logic_manage_name(logic_manage_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
logic_manage_name_hs(logic_manage_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

ptr_int_t logic_manage_tick(void * ctx, ptr_int_t max_process_count, float delta) {
    logic_manage_t mgr;
    ptr_int_t processed_count = 0;
    uint32_t queue_process_count;
    uint32_t i;

    mgr = (logic_manage_t)ctx;

    if (mgr->m_debug >= 4) {
        CPE_INFO(
            gd_app_em(mgr->m_app), "%s: tick: waiting-count=%d, pending-count=%d",
            logic_manage_name(mgr), mgr->m_waiting_count, mgr->m_pending_count);
    }

    queue_process_count = mgr->m_pending_count;
    if (queue_process_count > max_process_count) {
        queue_process_count = (uint32_t)max_process_count;
    }

    for(i = 0; i < queue_process_count; ++i, ++processed_count) {
        logic_context_t context = TAILQ_FIRST(&mgr->m_pending_contexts);
        logic_context_execute(context); /*dequeue in context !!!*/
    }

    return processed_count;
}

EXPORT_DIRECTIVE
int logic_manage_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    logic_manage_t logic_manage;

    logic_manage = logic_manage_create(
        app,
        gd_timer_mgr_find_nc(app, cfg_get_string(cfg, "timer-mgr", NULL)),
        gd_app_module_name(module), gd_app_alloc(app));
    if (logic_manage == NULL) return -1;

    logic_manage->m_debug = cfg_get_int32(cfg, "debug", 0);
    logic_manage->m_require_timout_ms = cfg_get_uint64(cfg, "require-timeout-ms", logic_manage->m_require_timout_ms);
    logic_manage->m_context_timout_ms = cfg_get_uint64(cfg, "context-timeout-ms", logic_manage->m_context_timout_ms);

    if (logic_manage->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            logic_manage_name(logic_manage));
    }

    return 0;
}

EXPORT_DIRECTIVE
void logic_manage_app_fini(gd_app_context_t app, gd_app_module_t module) {
    logic_manage_t logic_manage;

    logic_manage = logic_manage_find_nc(app, gd_app_module_name(module));
    if (logic_manage) {
        logic_manage_free(logic_manage);
    }
}

