#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "svr/center/agent/center_agent.h"
#include "center_agent_internal_ops.h"

EXPORT_DIRECTIVE
int center_agent_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    center_agent_t center_agent;

    center_agent =
        center_agent_create(
            app, gd_app_module_name(module),
            gd_app_alloc(app), gd_app_em(app));
    if (center_agent == NULL) return -1;

    center_agent->m_debug = cfg_get_int8(cfg, "debug", center_agent->m_debug);

    if (center_agent->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void center_agent_app_fini(gd_app_context_t app, gd_app_module_t module) {
    center_agent_t center_agent;

    center_agent = center_agent_find_nc(app, gd_app_module_name(module));
    if (center_agent) {
        center_agent_free(center_agent);
    }
}
