#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/utils/stream.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/net/net_endpoint.h"
#include "cpe/fsm/fsm_def.h"
#include "cpe/fsm/fsm_ins.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "svr/center/agent/center_agent.h"
#include "center_agent_internal_ops.h"

static void center_agent_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_center_agent = {
    "svr_center_agent",
    center_agent_clear
};

center_agent_t
center_agent_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct center_agent * mgr;
    nm_node_t mgr_node;

    assert(app);

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct center_agent));
    if (mgr_node == NULL) return NULL;

    mgr = (center_agent_t)nm_node_data(mgr_node);

    mgr->m_app = app;
    mgr->m_alloc = alloc;
    mgr->m_em = em;
    mgr->m_debug = 0;

    if (cpe_hash_table_init(
            &mgr->m_svr_types,
            alloc,
            (cpe_hash_fun_t) center_agent_svr_type_hash,
            (cpe_hash_eq_t) center_agent_svr_type_eq,
            CPE_HASH_OBJ2ENTRY(center_agent_svr_type, m_hh),
            -1) != 0)
    {
        nm_node_free(mgr_node);
        return NULL;
    }

    mem_buffer_init(&mgr->m_dump_buffer, mgr->m_alloc);

    nm_node_set_type(mgr_node, &s_nm_node_type_center_agent);

    return mgr;
}

static void center_agent_clear(nm_node_t node) {
    center_agent_t mgr;
    mgr = (center_agent_t)nm_node_data(node);

    center_agent_svr_type_free_all(mgr);

    assert(cpe_hash_table_count(&mgr->m_svr_types) == 0);
    cpe_hash_table_fini(&mgr->m_svr_types);

    mem_buffer_clear(&mgr->m_dump_buffer);
}

gd_app_context_t center_agent_app(center_agent_t mgr) {
    return mgr->m_app;
}

void center_agent_free(center_agent_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_center_agent) return;
    nm_node_free(mgr_node);
}

center_agent_t
center_agent_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_center_agent) return NULL;
    return (center_agent_t)nm_node_data(node);
}

center_agent_t
center_agent_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) name = "center_agent";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_center_agent) return NULL;
    return (center_agent_t)nm_node_data(node);
}

const char * center_agent_name(center_agent_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
center_agent_name_hs(center_agent_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}
