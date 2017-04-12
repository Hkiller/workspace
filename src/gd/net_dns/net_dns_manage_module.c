#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "net_dns_manage_i.h"

EXPORT_DIRECTIVE
int net_dns_manage_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    net_dns_manage_t net_dns_manage;

    net_dns_manage =
        net_dns_manage_create(
            app, gd_app_module_name(module),
            gd_app_alloc(app), gd_app_em(app));
    if (net_dns_manage == NULL) return -1;

    net_dns_manage->m_debug = cfg_get_int8(cfg, "debug", net_dns_manage->m_debug);

    if (net_dns_manage->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void net_dns_manage_app_fini(gd_app_context_t app, gd_app_module_t module) {
    net_dns_manage_t net_dns_manage;

    net_dns_manage = net_dns_manage_find_nc(app, gd_app_module_name(module));
    if (net_dns_manage) {
        net_dns_manage_free(net_dns_manage);
    }
}
