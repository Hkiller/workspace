#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/uhub/agent/uhub_agent.h"
#include "uhub_agent_internal_ops.h"

EXPORT_DIRECTIVE
int uhub_agent_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    uhub_agent_t uhub_agent;
    const char * str_uhub_svr_name;
    set_svr_svr_info_t uhub_svr_info;

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    str_uhub_svr_name = cfg_get_string(cfg, "uhub-svr", NULL);
    if (str_uhub_svr_name == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: uhub-svr not configured", gd_app_module_name(module));
        return -1;
    }

    if (str_uhub_svr_name[0] == '$') {
        char arg_name_buff[128];
        snprintf(arg_name_buff, sizeof(arg_name_buff), "--%s", str_uhub_svr_name + 1);
        str_uhub_svr_name = gd_app_arg_find(app, arg_name_buff);
        if (str_uhub_svr_name == NULL) {
            CPE_ERROR(gd_app_em(app), "%s: create: %s not configured!", gd_app_module_name(module), arg_name_buff);
            return -1;
        }
    }

    uhub_svr_info = set_svr_svr_info_find_by_name(stub, str_uhub_svr_name);
    if (uhub_svr_info == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: uhub-svr %s not exist!",
            gd_app_module_name(module), str_uhub_svr_name);
        return -1;
    }
    
    uhub_agent =
        uhub_agent_create(
            app, gd_app_module_name(module), stub, set_svr_svr_info_svr_type_id(uhub_svr_info),
            gd_app_alloc(app), gd_app_em(app));
    if (uhub_agent == NULL) return -1;

    uhub_agent->m_debug = cfg_get_int8(cfg, "debug", uhub_agent->m_debug);

    if (uhub_agent->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done. uhub-svr-type=%s(%d)",
            gd_app_module_name(module),
            set_svr_svr_info_svr_type_name(uhub_svr_info),
            set_svr_svr_info_svr_type_id(uhub_svr_info));
    }

    return 0;
}

EXPORT_DIRECTIVE
void uhub_agent_app_fini(gd_app_context_t app, gd_app_module_t module) {
    uhub_agent_t uhub_agent;

    uhub_agent = uhub_agent_find_nc(app, gd_app_module_name(module));
    if (uhub_agent) {
        uhub_agent_free(uhub_agent);
    }
}
