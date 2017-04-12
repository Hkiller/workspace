#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/uhub/agent/uhub_agent.h"
#include "uhub_agent_internal_ops.h"

static void uhub_agent_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_uhub_agent = {
    "svr_uhub_agent",
    uhub_agent_clear
};

uhub_agent_t
uhub_agent_create(
    gd_app_context_t app,
    const char * name, set_svr_stub_t stub, uint16_t uhub_svr_type,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct uhub_agent * mgr;
    nm_node_t mgr_node;

    assert(app);

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct uhub_agent));
    if (mgr_node == NULL) return NULL;

    mgr = (uhub_agent_t)nm_node_data(mgr_node);

    mgr->m_app = app;
    mgr->m_alloc = alloc;
    mgr->m_em = em;
    mgr->m_debug = 0;
    mgr->m_stub = stub;
    mgr->m_uhub_svr_type = uhub_svr_type;

    nm_node_set_type(mgr_node, &s_nm_node_type_uhub_agent);

    return mgr;
}

static void uhub_agent_clear(nm_node_t node) {
    uhub_agent_t mgr;
    mgr = (uhub_agent_t)nm_node_data(node);
}

gd_app_context_t uhub_agent_app(uhub_agent_t mgr) {
    return mgr->m_app;
}

void uhub_agent_free(uhub_agent_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_uhub_agent) return;
    nm_node_free(mgr_node);
}

uhub_agent_t
uhub_agent_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_uhub_agent) return NULL;
    return (uhub_agent_t)nm_node_data(node);
}

uhub_agent_t
uhub_agent_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) name = "uhub_agent";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_uhub_agent) return NULL;
    return (uhub_agent_t)nm_node_data(node);
}

const char * uhub_agent_name(uhub_agent_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
uhub_agent_name_hs(uhub_agent_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

int uhub_agent_send_notify_pkg(
    uhub_agent_t agent, dp_req_t body,
    void const * carry_data, size_t carry_data_len)
{
    return set_svr_stub_send_notify_pkg(
        agent->m_stub, agent->m_uhub_svr_type, 0,
        0, body, carry_data, carry_data_len);
}

int uhub_agent_send_notify_data(
    uhub_agent_t agent,
    void const * data, uint16_t data_size, LPDRMETA meta,
    void const * carry_data, size_t carry_data_len)
{
    return set_svr_stub_send_notify_data(
        agent->m_stub, agent->m_uhub_svr_type, 0,
        0, data, data_size, meta,
        carry_data, carry_data_len);
}
