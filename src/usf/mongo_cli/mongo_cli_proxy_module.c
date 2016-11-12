#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_manage.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "mongo_cli_internal_ops.h"

EXPORT_DIRECTIVE
int mongo_cli_proxy_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    mongo_cli_proxy_t proxy;
    logic_manage_t logic_manage;
    mongo_driver_t mongo_driver;
    const char * mongo_driver_name;
    const char * outgoing_send_to;
    const char * incoming_recv_at;
    const char * dft_db;

    outgoing_send_to = cfg_get_string(cfg, "outgoing-send-to", NULL);
    if (outgoing_send_to == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: outgoing-send-to not configured!",
            gd_app_module_name(module));
        return -1;
    }

    incoming_recv_at = cfg_get_string(cfg, "incoming-recv-at", NULL);
    if (incoming_recv_at == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: incoming-recv-at not configured!",
            gd_app_module_name(module));
        return -1;
    }

    logic_manage = logic_manage_find_nc(app, cfg_get_string(cfg, "logic-manage", NULL));
    if (logic_manage == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: logic-manage %s not exist!",
            gd_app_module_name(module),
            cfg_get_string(cfg, "logic-manage", "default"));
        return -1;
    }

    mongo_driver_name = cfg_get_string(cfg, "mongo-driver", NULL);
    if (mongo_driver_name == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: mongo-driver not configured!",
            gd_app_module_name(module));
        return -1;
    }

    mongo_driver = mongo_driver_find_nc(app, mongo_driver_name);
    if (mongo_driver == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: mongo-driver %s not exist!",
            gd_app_module_name(module), mongo_driver_name);
        return -1;
    }

    proxy = mongo_cli_proxy_create(app, gd_app_module_name(module), logic_manage, mongo_driver, gd_app_alloc(app), gd_app_em(app));
    if (proxy == NULL) return -1;

    if (mongo_cli_proxy_set_outgoing_send_to(proxy, outgoing_send_to) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set outgoing-sent-to %s fail!", 
            gd_app_module_name(module), outgoing_send_to);
        mongo_cli_proxy_free(proxy);
        return -1;
    }

    if (mongo_cli_proxy_set_incoming_recv_at(proxy, incoming_recv_at) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set incoming-recv-at %s fail!", 
            gd_app_module_name(module), incoming_recv_at);
        mongo_cli_proxy_free(proxy);
        return -1;
    }

    dft_db = cfg_get_string(cfg, "dft-db", NULL);
    if (dft_db) {
        if (dft_db[0] == '$') {
            char arg_name_buf[128];
            snprintf(arg_name_buf, sizeof(arg_name_buf), "--%s", dft_db + 1);
            dft_db = gd_app_arg_find(app, arg_name_buf);
            if (dft_db == NULL) {
                CPE_ERROR(
                    gd_app_em(app), "%s: create: dft-db arg %s not configured!", 
                    gd_app_module_name(module), arg_name_buf);
                mongo_cli_proxy_free(proxy);
                return -1;
            }
        }

        mongo_cli_proxy_set_dft_db(proxy, dft_db);
    }

    proxy->m_pkg_buf_max_size = cfg_get_uint32(cfg, "buf-size", proxy->m_pkg_buf_max_size);
    proxy->m_debug = cfg_get_int32(cfg, "debug", proxy->m_debug);

    return 0;
}

EXPORT_DIRECTIVE
void mongo_cli_proxy_app_fini(gd_app_context_t app, gd_app_module_t module) {
    mongo_cli_proxy_t proxy;

    proxy = mongo_cli_proxy_find_nc(app, gd_app_module_name(module));
    if (proxy) {
        mongo_cli_proxy_free(proxy);
    }
}
