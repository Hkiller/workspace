#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/stream_error.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/tl/tl_action.h"
#include "cpe/timer/timer_manage.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/app/app_tl.h"
#include "gd/timer/timer_manage.h"
#include "timer_internal_ops.h"

cpe_hash_string_t s_gd_timer_mgr_default_name;
struct nm_node_type s_nm_node_type_gd_timer_mgr;

gd_timer_mgr_t
gd_timer_mgr_create(
    gd_app_context_t app,
    const char * name,
    const char * tl_name,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    gd_timer_mgr_t mgr;
    tl_manage_t tl_mgr;
    nm_node_t mgr_node;

    if (name == 0) name = cpe_hs_data(s_gd_timer_mgr_default_name);

    if (em == NULL) em = gd_app_em(app);

    mgr_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct gd_timer_mgr));
    if (mgr_node == NULL) return NULL;

    mgr = (gd_timer_mgr_t)nm_node_data(mgr_node);
    mgr->m_alloc = alloc;
    mgr->m_app = app;
    mgr->m_em = em;
    mgr->m_debug = 0;

    tl_mgr = app_tl_manage_find(app, tl_name);
    if (tl_mgr == NULL) {
        CPE_ERROR(em, "gd_timer_mgr_create: tl %s not exist!", tl_name ? tl_name : "default");
        nm_node_free(mgr_node);
        return NULL;
    }

    mgr->m_timer_mgr = cpe_timer_mgr_create(tl_mgr, alloc, em);
    if (mgr->m_timer_mgr == NULL) {
        CPE_ERROR(em, "gd_timer_mgr_create: create cpe timer manager fail!");
        nm_node_free(mgr_node);
        return NULL;
    }

    nm_node_set_type(mgr_node, &s_nm_node_type_gd_timer_mgr);

    return mgr;
}

static void gd_timer_mgr_clear(nm_node_t node) {
    gd_timer_mgr_t mgr;
    mgr = (gd_timer_mgr_t)nm_node_data(node);

    cpe_timer_mgr_free(mgr->m_timer_mgr);
}

void gd_timer_mgr_free(gd_timer_mgr_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_gd_timer_mgr) return;
    nm_node_free(mgr_node);
}

gd_timer_mgr_t gd_timer_mgr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    if (name == NULL) name = s_gd_timer_mgr_default_name;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_gd_timer_mgr) return NULL;
    return (gd_timer_mgr_t)nm_node_data(node);
}

gd_timer_mgr_t gd_timer_mgr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;
    if (name == NULL) return gd_timer_mgr_default(app);

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_gd_timer_mgr) return NULL;
    return (gd_timer_mgr_t)nm_node_data(node);
}

gd_timer_mgr_t gd_timer_mgr_default(gd_app_context_t app) {
    return gd_timer_mgr_find(app, s_gd_timer_mgr_default_name);
}

gd_app_context_t gd_timer_mgr_app(gd_timer_mgr_t mgr) {
    return mgr->m_app;
}

const char * gd_timer_mgr_name(gd_timer_mgr_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
gd_timer_mgr_name_hs(gd_timer_mgr_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

tl_t gd_timer_mgr_tl(gd_timer_mgr_t mgr) {
    return cpe_timer_mgr_tl(mgr->m_timer_mgr);
}

int gd_timer_mgr_regist_timer(
    gd_timer_mgr_t mgr, 
    gd_timer_id_t * id,
    gd_timer_process_fun_t fun, void * ctx,
    void * arg, void (*arg_fini)(void *),
    tl_time_span_t delay, tl_time_span_t span, int repeatCount)
{
    return cpe_timer_mgr_regist_timer(mgr->m_timer_mgr, id, fun, ctx, arg, arg_fini, delay, span, repeatCount);
}

void gd_timer_mgr_unregist_timer_by_ctx(gd_timer_mgr_t mgr, void * ctx) {
    cpe_timer_mgr_unregist_timer_by_ctx(mgr->m_timer_mgr, ctx);
}

void gd_timer_mgr_unregist_timer_by_id(gd_timer_mgr_t mgr, gd_timer_id_t timer_id) {
    cpe_timer_mgr_unregist_timer_by_id(mgr->m_timer_mgr, timer_id);
}

int gd_timer_mgr_have_timer(gd_timer_mgr_t mgr, gd_timer_id_t timer_id) {
    return cpe_timer_mgr_have_timer(mgr->m_timer_mgr, timer_id);
}

CPE_HS_DEF_VAR(s_gd_timer_mgr_default_name, "gd_timer_mgr");

struct nm_node_type s_nm_node_type_gd_timer_mgr = {
    "gd_timer_mgr",
    gd_timer_mgr_clear
};

EXPORT_DIRECTIVE
int gd_timer_mgr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    gd_timer_mgr_t gd_timer_mgr;

    gd_timer_mgr =
        gd_timer_mgr_create(
            app,
            gd_app_module_name(module),
            cfg_get_string(cfg, "tl", NULL),
            gd_app_alloc(app),
            gd_app_em(app));
    if (gd_timer_mgr == NULL) return -1;

    gd_timer_mgr->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (gd_timer_mgr->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            gd_timer_mgr_name(gd_timer_mgr));
    }

    return 0;
}

EXPORT_DIRECTIVE
void gd_timer_mgr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    gd_timer_mgr_t gd_timer_mgr;

    gd_timer_mgr = gd_timer_mgr_find_nc(app, gd_app_module_name(module));
    if (gd_timer_mgr) {
        gd_timer_mgr_free(gd_timer_mgr);
    }
}

