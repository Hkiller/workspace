#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "net_trans_manage_i.h"

static int net_trans_manage_curl_init(error_monitor_t em, cfg_t cfg);
static void net_trans_manage_curl_fini(error_monitor_t em);

EXPORT_DIRECTIVE
int net_trans_manage_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    net_trans_manage_t net_trans_manage;

    if (net_trans_manage_curl_init(gd_app_em(app), cfg_find_cfg(cfg, "init")) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: init curl fail!", gd_app_module_name(module));
        return -1;
    }

    net_trans_manage =
        net_trans_manage_create(
            app, gd_app_module_name(module),
            gd_app_alloc(app), gd_app_em(app));
    if (net_trans_manage == NULL) return -1;

    net_trans_manage->m_debug = cfg_get_int8(cfg, "debug", net_trans_manage->m_debug);

    if (net_trans_mult_handler_init(net_trans_manage) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: init mult handler fail!", gd_app_module_name(module));
        net_trans_manage_curl_fini(gd_app_em(app));
        net_trans_manage_free(net_trans_manage);
        return -1;
    }

    if (net_trans_manage->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void net_trans_manage_app_fini(gd_app_context_t app, gd_app_module_t module) {
    net_trans_manage_t net_trans_manage;

    net_trans_manage = net_trans_manage_find_nc(app, gd_app_module_name(module));
    if (net_trans_manage) {
        net_trans_manage_free(net_trans_manage);
    }

    net_trans_manage_curl_fini(gd_app_em(app));
}

static int g_curl_init = 0;

static int net_trans_manage_curl_init(error_monitor_t em, cfg_t cfg) {
    if (g_curl_init == 0) {
        int rc;

        rc = curl_global_init(CURL_GLOBAL_ALL);
        if (rc != 0) {
            CPE_ERROR(em, "curl_global_init fail, rv=%d", rc);
            return -1;
        }

        CPE_INFO(em, "curl_global_init success");
    }

    ++g_curl_init;
    return 0;
}

static void net_trans_manage_curl_fini(error_monitor_t em) {
    if (--g_curl_init == 0) {
        curl_global_cleanup();
        CPE_INFO(em, "curl_global_cleanup success");
    }
}
