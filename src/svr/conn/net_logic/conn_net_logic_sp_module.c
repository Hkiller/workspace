#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "usf/logic/logic_manage.h"
#include "svr/conn/net_logic/conn_net_logic_sp.h"
#include "conn_net_logic_internal_ops.h"

EXPORT_DIRECTIVE
int conn_net_logic_sp_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    conn_net_logic_sp_t sp;
    logic_manage_t logic_manage;
    const char * outgoing_dispatch_to;
    const char * incoming_recv_at;

    logic_manage = logic_manage_find_nc(app, cfg_get_string(cfg, "logic-manage", NULL));
    if (logic_manage == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: logic-manage %s not exist!",
            gd_app_module_name(module),
            cfg_get_string(cfg, "logic-manage", "default"));
        return -1;
    }


    outgoing_dispatch_to = cfg_get_string(cfg, "outgoing-send-to", NULL);
    if (outgoing_dispatch_to == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: outgoing-send-to not configured!", gd_app_module_name(module));
        return -1;
    }

    incoming_recv_at = cfg_get_string(cfg, "incoming-recv-at", NULL);
    if (incoming_recv_at == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: incoming-recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    sp = conn_net_logic_sp_create(
        app, gd_app_module_name(module),
        logic_manage, gd_app_alloc(app), gd_app_em(app));
    if (sp == NULL) return -1;

    sp->m_debug = cfg_get_int8(cfg, "debug", sp->m_debug);

    if (conn_net_logic_sp_set_outgoing_dispatch_to(sp, outgoing_dispatch_to) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set outgoing-send-to %s fail!",
            gd_app_module_name(module), outgoing_dispatch_to);
        conn_net_logic_sp_free(sp);
        return -1;
    }
    
    if (conn_net_logic_sp_set_incoming_recv_at(sp, incoming_recv_at) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set incoming-recv-at %s fail!",
            gd_app_module_name(module), incoming_recv_at);
        conn_net_logic_sp_free(sp);
        return -1;
    }

    if (sp->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void conn_net_logic_sp_app_fini(gd_app_context_t app, gd_app_module_t module) {
    conn_net_logic_sp_t conn_net_logic_sp;

    conn_net_logic_sp = conn_net_logic_sp_find_nc(app, gd_app_module_name(module));
    if (conn_net_logic_sp) {
        conn_net_logic_sp_free(conn_net_logic_sp);
    }
}
