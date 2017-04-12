#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/dblog/agent/dblog_agent.h"
#include "dblog_agent_i.h"

static int dblog_agent_load_target(gd_app_context_t app, dblog_agent_t agent, const char * value);

EXPORT_DIRECTIVE
int dblog_agent_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    dblog_agent_t agent;

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }


    agent = dblog_agent_create(
        app, gd_app_module_name(module),
        set_svr_stub_svr_id(stub),
        set_svr_svr_info_svr_type_id(set_svr_stub_svr_type(stub)),
        gd_app_alloc(app), gd_app_em(app));
    if (agent == NULL) return -1;

    agent->m_debug = cfg_get_int8(cfg, "debug", agent->m_debug);

    if (dblog_agent_load_target(app, agent, cfg_get_string(cfg, "target", NULL)) != 0) {
        dblog_agent_free(agent);
        return -1;
    }
    
    if (agent->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done. target=%s:%d", gd_app_module_name(module), agent->m_ip, agent->m_port);
    }

    return 0;
}

EXPORT_DIRECTIVE
void dblog_agent_app_fini(gd_app_context_t app, gd_app_module_t module) {
    dblog_agent_t dblog_agent;

    dblog_agent = dblog_agent_find_nc(app, gd_app_module_name(module));
    if (dblog_agent) {
        dblog_agent_free(dblog_agent);
    }
}

static int dblog_agent_load_target(gd_app_context_t app, dblog_agent_t agent, const char * value) {
    char * sep;
    int port;
    int host_len;
    char host[64];

    if (value[0] == '$') {
        char buff[64];
        snprintf(buff, sizeof(buff), "--%s", value + 1);

        value = gd_app_arg_find(app, buff);
        if (value == NULL) {
            if (agent->m_debug) {
                CPE_INFO(gd_app_em(app), "%s: create: read target: arg %s not exist!", dblog_agent_name(agent), buff);
            }
            return 0;
        }
    }
    
    sep = strchr(value, ':');
    if (sep == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: target %s format error!", dblog_agent_name(agent), value);
        return -1;
    }

    host_len = sep - value;
    if (host_len + 1 > sizeof(host)) {
        CPE_ERROR(gd_app_em(app), "%s: create: target %s host too long!", dblog_agent_name(agent), value);
        return -1;
    }

    memcpy(host, value, host_len);
    host[host_len] = 0;

    port = atoi(sep + 1);
        
    if (dblog_agent_set_target(agent, host, port) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set target %s:%d fail!", dblog_agent_name(agent), host, port);
        return -1;
    }

    return 0;
}
