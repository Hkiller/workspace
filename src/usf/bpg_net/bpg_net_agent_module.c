#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/net/net_listener.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "usf/bpg_net/bpg_net_agent.h"
#include "bpg_net_internal_ops.h"

EXPORT_DIRECTIVE
int bpg_net_agent_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    bpg_net_agent_t bpg_net_agent;
    const char * ip;
    short port;
    int accept_queue_size;
    bpg_pkg_manage_t pkg_manage;
    cfg_t reply_recv_cfg;

    pkg_manage = bpg_pkg_manage_find_nc(app, cfg_get_string(cfg, "pkg-manage", NULL));
    if (pkg_manage == NULL) {
        CPE_ERROR(
                gd_app_em(app), "%s: create: pkg-manage %s not exist!",
                gd_app_module_name(module),
                cfg_get_string(cfg, "pkg-manage", "default"));
        return -1;
    }

    reply_recv_cfg = cfg_find_cfg(cfg, "reply-recv-at");
    if (reply_recv_cfg == NULL) {
        CPE_ERROR(
                gd_app_em(app), "%s: create: reply-recv-at not configured!",
                gd_app_module_name(module));
        return -1;
    }

    ip = cfg_get_string(cfg, "ip", "");
    port = cfg_get_int16(cfg, "port", 0);
    accept_queue_size = cfg_get_int32(cfg, "accept-queue-size", 256);

    bpg_net_agent =
        bpg_net_agent_create(
                app, pkg_manage, gd_app_module_name(module),
                ip, port, accept_queue_size,
                gd_app_alloc(app), gd_app_em(app));
    if (bpg_net_agent == NULL) return -1;

    bpg_net_agent->m_req_max_size =
        cfg_get_uint32(cfg, "req-max-size", bpg_net_agent->m_req_max_size);
    bpg_net_agent->m_read_chanel_size =
        cfg_get_uint32(cfg, "read-chanel-size", bpg_net_agent->m_read_chanel_size);
    bpg_net_agent->m_write_chanel_size =
        cfg_get_uint32(cfg, "write-chanel-size", bpg_net_agent->m_write_chanel_size);

    bpg_net_agent->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (dp_rsp_bind_by_cfg(bpg_net_agent->m_reply_rsp, reply_recv_cfg, gd_app_em(app)) != 0) {
        CPE_ERROR(
                gd_app_em(app), "%s: create: bind rsp by cfg fail!",
                gd_app_module_name(module));
        bpg_net_agent_free(bpg_net_agent);
        return -1;
    }

    if (bpg_net_agent_set_dispatch_to(bpg_net_agent, cfg_get_string(cfg, "dispatch-to", NULL)) != 0) {
        CPE_ERROR(
                gd_app_em(app), "%s: create: set dispatch to fail!",
                gd_app_module_name(module));
        bpg_net_agent_free(bpg_net_agent);
        return -1;
    }

    bpg_net_agent_set_conn_timeout(
            bpg_net_agent, cfg_get_uint32(cfg, "conn-timeout", 0) * 1000);

    if (bpg_net_agent->m_debug) {
        CPE_INFO(
                gd_app_em(app),
                "%s: create: done. ip=%s, port=%u, accept-queue-size=%d, req-max-size=%d, pkg-manage=%s, timeout=%d(ms)",
                gd_app_module_name(module),
                ip, bpg_net_agent_port(bpg_net_agent),
                accept_queue_size, (int)bpg_net_agent->m_req_max_size,
                bpg_pkg_manage_name(pkg_manage),
                (int)bpg_net_agent->m_conn_timeout);
    }

    return 0;
}

EXPORT_DIRECTIVE
void bpg_net_agent_app_fini(gd_app_context_t app, gd_app_module_t module) {
    bpg_net_agent_t bpg_net_agent;

    bpg_net_agent = bpg_net_agent_find_nc(app, gd_app_module_name(module));
    if (bpg_net_agent) {
        bpg_net_agent_free(bpg_net_agent);
    }
}
