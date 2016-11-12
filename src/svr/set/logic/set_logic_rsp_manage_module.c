#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/app/app_library.h"
#include "usf/logic/logic_manage.h"
#include "usf/logic/logic_executor_mgr.h"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/set/logic/set_logic_rsp_manage.h"
#include "svr/set/logic/set_logic_rsp.h"
#include "set_logic_rsp_ops.h"

static int set_logic_rsp_manage_load_commit_dsp(
    gd_app_context_t app, gd_app_module_t module, set_logic_rsp_manage_t mgr, cfg_t cfg)
{
    const char * commit_to;

    if ((commit_to = cfg_get_string(cfg, "commit-to", NULL))) {
        mgr->m_commit_to = cpe_hs_create(mgr->m_alloc, commit_to);
        if (mgr->m_commit_to == NULL) {
            CPE_ERROR(gd_app_em(app), "%s: create: set-commit-to fail", gd_app_module_name(module));
            return -1;
        }
    }

    return 0;
}

static int set_logic_rsp_manage_load_recv_at(
    gd_app_context_t app, gd_app_module_t module, set_logic_rsp_manage_t mgr, cfg_t cfg)
{
    const char * dsp_recv_at;

    if ((dsp_recv_at = cfg_get_string(cfg, "recv-at", NULL))) {
        if (set_logic_rsp_manage_set_recv_at(mgr, dsp_recv_at) != 0) {
            CPE_ERROR(gd_app_em(app), "%s: create: set-recv-at fail", gd_app_module_name(module));
            return -1;
        }
    }

    return 0;
}

static int set_logic_rsp_manage_load_queue_infos(
    gd_app_context_t app, gd_app_module_t module, set_logic_rsp_manage_t mgr, cfg_t cfg)
{
    struct cfg_it queue_it;
    cfg_t queue_cfg;

    cfg_it_init(&queue_it, cfg_find_cfg(cfg, "logic-queue"));

    while((queue_cfg = cfg_it_next(&queue_it))) {
        const char * name;
        const char * scope_name;
        set_logic_rsp_queue_scope_t scope;
        struct set_logic_rsp_queue_info * queue;

        name = cfg_get_string(queue_cfg, "name", NULL);
        if (name == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: create logic queue info: no name!",
                gd_app_module_name(module));
            return -1;
        }

        scope_name = cfg_get_string(queue_cfg, "scope", NULL);
        if (scope_name == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: create logic queue %s: scope not configured!",
                gd_app_module_name(module), name);
            return -1;
        }

        if (strcmp(scope_name, "global") == 0) {
            scope = set_logic_rsp_queue_scope_global;
        }
        else if (strcmp(scope_name, "client") == 0) {
            scope = set_logic_rsp_queue_scope_client;
        }
        else {
            CPE_ERROR(
                gd_app_em(app), "%s: create: create logic queue %s: scope not configured!",
                gd_app_module_name(module), name);
            return -1;
        }

        queue = set_logic_rsp_queue_info_create(
            mgr, name, scope,
            cfg_get_uint32(queue_cfg, "max-count", 0));
        if (queue == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: create logic queue %s: create fail!",
                gd_app_module_name(module), name);
            return -1;
        }

        if (cfg_get_int32(queue_cfg, "is-default", 0)) {
            if (mgr->m_default_queue_info != NULL) {
                CPE_ERROR(
                    gd_app_em(app), "%s: create: create logic queue %s: default queue info already exist, it is %s!",
                    gd_app_module_name(module), name, set_logic_rsp_queue_name(mgr->m_default_queue_info));
                return -1;
            }
            else {
                mgr->m_default_queue_info = queue;
            }
        }

        if (mgr->m_debug) {
            CPE_INFO(
                gd_app_em(app), "%s: create: create logic queue %s: scope=%s, max-count=%d!",
                gd_app_module_name(module), cpe_hs_data(queue->m_name), scope_name, queue->m_max_count);
        }
    }

    if (mgr->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: default queue %s!",
            gd_app_module_name(module), 
            mgr->m_default_queue_info
            ? cpe_hs_data(mgr->m_default_queue_info->m_name)
            : "none");
    }

    return 0;
}

EXPORT_DIRECTIVE
int set_logic_rsp_manage_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_logic_rsp_manage_t mgr;
    set_logic_rsp_manage_dp_scope_t scope;
    const char * str_scope;
    logic_manage_t logic_mgr;
    logic_executor_mgr_t executor_mgr;
    set_svr_stub_t stub;
    cfg_t child_cfg;
    const char * executor_mgr_name;
    const char * load_from;
    const char * queue_attr;

    str_scope = cfg_get_string(cfg, "scope", "global");
    if (strcmp(str_scope, "global") == 0) {
        scope = set_logic_rsp_manage_dp_scope_global;
    }
    else if (strcmp(str_scope, "local") == 0) {
        scope = set_logic_rsp_manage_dp_scope_local;
    }
    else {
        CPE_ERROR(
            gd_app_em(app),
            "%s: create: scope %s is unknown!",
            gd_app_module_name(module), str_scope);
        return -1;
    }

    logic_mgr = logic_manage_find_nc(app, cfg_get_string(cfg, "logic-manage", NULL));
    if (logic_mgr == NULL) {
        CPE_ERROR(
            gd_app_em(app),
            "%s: create: logic-manage %s not exist",
            gd_app_module_name(module), cfg_get_string(cfg, "logic-manage", "default"));
        return -1;
    }

    executor_mgr_name = cfg_get_string(cfg, "executor-manage", NULL);
    if (executor_mgr_name == NULL) {
        CPE_ERROR(
            gd_app_em(app),
            "%s: create: executor-manage not configured",
            gd_app_module_name(module));
        return -1;
    }

    executor_mgr = logic_executor_mgr_find_nc(app, executor_mgr_name);
    if (executor_mgr == NULL) {
        CPE_ERROR(
            gd_app_em(app),
            "%s: create: executor-manage %s not exist",
            gd_app_module_name(module), executor_mgr_name);
        return -1;
    }

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app),
            "%s: create: set-svr-stub %s not exist",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    mgr = 
        set_logic_rsp_manage_create(
            app,
            gd_app_module_name(module),
            scope,
            logic_mgr,
            executor_mgr,
            stub,
            NULL);
    if (mgr == NULL) {
        return -1;
    }

    mgr->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (set_logic_rsp_manage_load_commit_dsp(app, module, mgr, cfg) != 0) {
        set_logic_rsp_manage_free(mgr);
        return -1;
    }

    if (set_logic_rsp_manage_load_recv_at(app, module, mgr, cfg) != 0) {
        set_logic_rsp_manage_free(mgr);
        return -1;
    }

    if (set_logic_rsp_manage_load_queue_infos(app, module, mgr, cfg) != 0) {
        set_logic_rsp_manage_free(mgr);
        return -1;
    }

    if ((queue_attr = cfg_get_string(cfg, "queue-attr", NULL))) {
        assert(mgr->m_queue_attr == NULL);
        mgr->m_queue_attr = cpe_str_mem_dup(mgr->m_alloc, queue_attr);
        if (mgr->m_queue_attr == NULL) {
            CPE_ERROR(gd_app_em(app), "%s: create: set queue-attr %s fail", gd_app_module_name(module), queue_attr);
            set_logic_rsp_manage_free(mgr);
            return -1;
        }
    }

    if ((load_from = cfg_get_string(cfg, "rsps-load-from", NULL))) {
        if (*load_from == '/') {
            child_cfg = cfg_find_cfg(gd_app_cfg(app), load_from + 1);
        }
        else {
            child_cfg = cfg_find_cfg(cfg, load_from);
        }
    }
    else {
        child_cfg = cfg_find_cfg(cfg, "rsps");
    }

    if (child_cfg) {
        const char * rsps_metalib_name = cfg_get_string(cfg, "rsps-load-meta", NULL);
        dr_store_t rsps_metalib = NULL;

        if (rsps_metalib_name) {
            const char * dr_store_manage_name= cfg_get_string(cfg, "rsp-load-meta-store-manage", NULL);
            dr_store_manage_t dr_store_mgr = dr_store_manage_find_nc(app, dr_store_manage_name);
            if (dr_store_mgr == NULL) {
                CPE_ERROR(
                    gd_app_em(app),
                    "%s: create: respons-load-metalib-store-manage %s not exist",
                    gd_app_module_name(module), dr_store_manage_name ? dr_store_manage_name : "default");
                set_logic_rsp_manage_free(mgr);
                return -1;
            }

            rsps_metalib = dr_store_find(dr_store_mgr, rsps_metalib_name);
            if (rsps_metalib == NULL) {
                CPE_ERROR(
                    gd_app_em(app),
                    "%s: create: metalib %s not exist in dr-store-manage %s",
                    gd_app_module_name(module), 
                    rsps_metalib_name,
                    dr_store_manage_name ? dr_store_manage_name : "default");
                set_logic_rsp_manage_free(mgr);
                return -1;
            }
        }

        if (set_logic_rsp_build(mgr, child_cfg, rsps_metalib ? dr_store_lib(rsps_metalib) : NULL, gd_app_em(app)) != 0) {
            set_logic_rsp_manage_free(mgr);
            return -1;
        }
    }

    if (mgr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void set_logic_rsp_manage_app_fini(gd_app_context_t app, gd_app_module_t module) {
    set_logic_rsp_manage_t mgr;

    mgr = set_logic_rsp_manage_find_nc(app, gd_app_module_name(module));
    if (mgr) {
        set_logic_rsp_manage_free(mgr);
    }
}
