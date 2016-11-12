#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_external.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/net_trans/net_trans_manage.h"
#include "gd/net_trans/net_trans_group.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "apple_iap_svr_ops.h"

EXPORT_DIRECTIVE
int apple_iap_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    net_trans_manage_t trans_mgr;
    apple_iap_svr_t apple_iap_svr;
    const char * send_to;
    const char * request_recv_at;
    const char * is_sandbox;

    if ((is_sandbox = gd_app_arg_find(app, "--is-sandbox")) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: is-sandbox not configured in args!", gd_app_module_name(module));
        return -1;
    }

    if ((send_to = cfg_get_string(cfg, "send-to", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: send-to not configured!", gd_app_module_name(module));
        return -1;
    }

    if ((request_recv_at = cfg_get_string(cfg, "request-recv-at", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: request-recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    trans_mgr = net_trans_manage_find_nc(app, cfg_get_string(cfg, "trans-mgr", NULL));
    if (trans_mgr == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: trans-mgr %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "trans-mgr", "default"));
        return -1;
    }

    apple_iap_svr =
        apple_iap_svr_create(
            app, gd_app_module_name(module),
            trans_mgr, stub, gd_app_alloc(app), gd_app_em(app));
    if (apple_iap_svr == NULL) return -1;

    apple_iap_svr->m_debug = cfg_get_int8(cfg, "debug", apple_iap_svr->m_debug);
    apple_iap_svr->m_is_sandbox = atoi(is_sandbox);

    net_trans_group_set_connect_timeout(apple_iap_svr->m_trans_group, 10 * 1000);
    net_trans_group_set_transfer_timeout(apple_iap_svr->m_trans_group, 10 * 1000);
    net_trans_group_set_forbid_reuse(apple_iap_svr->m_trans_group, 1);

    if (apple_iap_svr_set_send_to(apple_iap_svr, send_to) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set send-to %s fail!", gd_app_module_name(module), send_to);
        apple_iap_svr_free(apple_iap_svr);
        return -1;
    }

    if (apple_iap_svr_set_request_recv_at(apple_iap_svr, request_recv_at) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set request-recv-at %s fail!", gd_app_module_name(module), request_recv_at);
        apple_iap_svr_free(apple_iap_svr);
        return -1;
    }

    if (apple_iap_svr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done. is-sandbox=%d", gd_app_module_name(module), apple_iap_svr->m_is_sandbox);
    }

    return 0;
}

EXPORT_DIRECTIVE
void apple_iap_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    apple_iap_svr_t apple_iap_svr;

    apple_iap_svr = apple_iap_svr_find_nc(app, gd_app_module_name(module));
    if (apple_iap_svr) {
        apple_iap_svr_free(apple_iap_svr);
    }
}

