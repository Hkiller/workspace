#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "svr/conn/net_cli/conn_net_cli.h"
#include "conn_net_bpg_internal_ops.h"

EXPORT_DIRECTIVE
int conn_net_bpg_chanel_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    conn_net_bpg_chanel_t sp;
    bpg_pkg_manage_t bpg_pkg_manage;
    conn_net_cli_t conn_net_cli;
    const char * incoming_dispatch_to;
    const char * incoming_recv_at;
    const char * outgoing_dispatch_to;
    const char * outgoing_recv_at;

    bpg_pkg_manage = bpg_pkg_manage_find_nc(app, cfg_get_string(cfg, "pkg-manage", NULL));
    if (bpg_pkg_manage == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: pkg-manage %s not exist!",
            gd_app_module_name(module),
            cfg_get_string(cfg, "pkg-manage", NULL));
        return -1;
    }

    conn_net_cli = conn_net_cli_find_nc(app, cfg_get_string(cfg, "conn-cli", NULL));
    if (conn_net_cli == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: conn-cli %s not exist!",
            gd_app_module_name(module),
            cfg_get_string(cfg, "conn-cli", NULL));
        return -1;
    }

    incoming_dispatch_to = cfg_get_string(cfg, "incoming-send-to", NULL);
    if (incoming_dispatch_to == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: incoming-send-to not configured!", gd_app_module_name(module));
        return -1;
    }

    incoming_recv_at = cfg_get_string(cfg, "incoming-recv-at", NULL);
    if (incoming_recv_at == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: incoming-recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    outgoing_recv_at = cfg_get_string(cfg, "outgoing-recv-at", NULL);
    if (outgoing_recv_at == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: outgoing-recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    outgoing_dispatch_to = cfg_get_string(cfg, "outgoing-send-to", NULL);
    if (outgoing_dispatch_to == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: outgoing-send-to not configured!", gd_app_module_name(module));
        return -1;
    }

    sp = conn_net_bpg_chanel_create(
        app, gd_app_module_name(module),
        bpg_pkg_manage, conn_net_cli, gd_app_alloc(app), gd_app_em(app));
    if (sp == NULL) return -1;

    sp->m_debug = cfg_get_int8(cfg, "debug", sp->m_debug);

    if (conn_net_bpg_chanel_set_incoming_recv_at(sp, incoming_recv_at) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set incoming-recv-at %s fail!",
            gd_app_module_name(module), incoming_recv_at);
        conn_net_bpg_chanel_free(sp);
        return -1;
    }

    if (conn_net_bpg_chanel_set_incoming_dispatch_to(sp, incoming_dispatch_to) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set incoming-send-to %s fail!",
            gd_app_module_name(module), incoming_dispatch_to);
        conn_net_bpg_chanel_free(sp);
        return -1;
    }

    if (conn_net_bpg_chanel_set_outgoing_recv_at(sp, outgoing_recv_at) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set outgoing-recv-at %s fail!",
            gd_app_module_name(module), outgoing_recv_at);
        conn_net_bpg_chanel_free(sp);
        return -1;
    }

    if (conn_net_bpg_chanel_set_outgoing_dispatch_to(sp, outgoing_dispatch_to) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set outgoing-send-to %s fail!",
            gd_app_module_name(module), outgoing_dispatch_to);
        conn_net_bpg_chanel_free(sp);
        return -1;
    }

    if (sp->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void conn_net_bpg_chanel_app_fini(gd_app_context_t app, gd_app_module_t module) {
    conn_net_bpg_chanel_t conn_net_bpg_chanel;

    conn_net_bpg_chanel = conn_net_bpg_chanel_find_nc(app, gd_app_module_name(module));
    if (conn_net_bpg_chanel) {
        conn_net_bpg_chanel_free(conn_net_bpg_chanel);
    }
}
