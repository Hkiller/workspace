#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "usf/logic/logic_manage.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "usf/bpg_cli/bpg_cli_proxy.h"
#include "bpg_cli_internal_types.h"

EXPORT_DIRECTIVE
int bpg_cli_proxy_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    bpg_cli_proxy_t bpg_cli_proxy;
    bpg_pkg_manage_t pkg_manage;
    logic_manage_t logic_manage;
    const char * recv_at;
    const char * str_buf_size;

    pkg_manage = bpg_pkg_manage_find_nc(app, cfg_get_string(cfg, "pkg-manage", NULL));
    if (pkg_manage == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: pkg-manage %s not exist!",
            gd_app_module_name(module),
            cfg_get_string(cfg, "pkg-manage", "default"));
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

    recv_at = cfg_get_string(cfg, "recv-at", NULL);
    if (recv_at == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: recv-at not configured!",
            gd_app_module_name(module));
        return -1;
    }

    bpg_cli_proxy =
        bpg_cli_proxy_create(
            app, gd_app_module_name(module),
            logic_manage,
            pkg_manage,
            gd_app_em(app));
    if (bpg_cli_proxy == NULL) return -1;

    if (bpg_cli_proxy_outgoing_set_recv_at(bpg_cli_proxy, recv_at) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set recv-at %s fail!",
            gd_app_module_name(module),
            recv_at);
        bpg_cli_proxy_free(bpg_cli_proxy);
        return -1;
    }

    if (bpg_cli_proxy_outgoing_set_send_to(bpg_cli_proxy, cfg_find_cfg(cfg, "send-to")) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set send-to fail!",
            gd_app_module_name(module));
        bpg_cli_proxy_free(bpg_cli_proxy);
        return -1;
    }

    if (bpg_cli_proxy_incoming_set_no_sn_send_to(bpg_cli_proxy, cfg_find_cfg(cfg, "incoming-no-sn-send-to")) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set incoming-no-sn-send-to fail!",
            gd_app_module_name(module));
        bpg_cli_proxy_free(bpg_cli_proxy);
        return -1;
    }

    if ((str_buf_size = cfg_get_string(cfg, "buf-size", NULL))) {
        bpg_cli_proxy->m_send_pkg_max_size = 
            (size_t)cpe_str_parse_byte_size_with_dft(str_buf_size, (uint64_t)bpg_cli_proxy->m_send_pkg_max_size);
    }

    bpg_cli_proxy->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (bpg_cli_proxy->m_debug) {
        CPE_INFO(
            gd_app_em(app),
            "%s: create: done. buf-max-size="FMT_SIZE_T", pkg-manage=%s, logic-manage=%s",
            gd_app_module_name(module),
            bpg_cli_proxy_buf_capacity(bpg_cli_proxy),
            bpg_pkg_manage_name(pkg_manage),
            logic_manage_name(logic_manage));
    }

    return 0;
}

EXPORT_DIRECTIVE
void bpg_cli_proxy_app_fini(gd_app_context_t app, gd_app_module_t module) {
    bpg_cli_proxy_t bpg_cli_proxy;

    bpg_cli_proxy = bpg_cli_proxy_find_nc(app, gd_app_module_name(module));
    if (bpg_cli_proxy) {
        bpg_cli_proxy_free(bpg_cli_proxy);
    }
}
