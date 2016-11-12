#include <assert.h>
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_context.h"
#include "log_internal_ops.h"

static void log_context_clear(nm_node_t node);

static cpe_hash_string_buf s_log_context_name = CPE_HS_BUF_MAKE("log_context");

struct nm_node_type s_nm_node_type_log_context = {
    "log_context",
    log_context_clear
};

struct log_context *
log_context_create(
    gd_app_context_t app,
    error_monitor_t em,
    mem_allocrator_t alloc)
{
    const char * name;
    struct log_context * mgr;
    nm_node_t mgr_node;

    assert(app);

    name = cpe_hs_data((cpe_hash_string_t)&s_log_context_name);

    if (em == 0) em = gd_app_em(app);

    mgr_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct log_context));
    if (mgr_node == NULL) return NULL;

    mgr = (struct log_context *)nm_node_data(mgr_node);

    mgr->m_app = app;
    mgr->m_em = em;
    mgr->m_alloc = alloc;
    mgr->m_debug = 0;

    TAILQ_INIT(&mgr->m_log4c_ems);

    nm_node_set_type(mgr_node, &s_nm_node_type_log_context);

    return mgr;
}

static void log_context_clear(nm_node_t node) {
    struct log_context * mgr;
    mgr = (struct log_context *)nm_node_data(node);
    log4c_em_free_all(mgr);
}

void log_context_free(struct log_context * mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_log_context) return;
    nm_node_free(mgr_node);
}

struct log_context *
log_context_find(gd_app_context_t app) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), (cpe_hash_string_t)&s_log_context_name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_log_context) return NULL;
    return (struct log_context *)nm_node_data(node);
}

gd_app_context_t log_context_app(struct log_context * mgr) {
    return mgr->m_app;
}

const char * log_context_name(struct log_context * mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
log_context_name_hs(struct log_context * mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}
