#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/app/app_library.h"
#include "usf/logic/logic_manage.h"
#include "usf/logic/logic_executor_mgr.h"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "usf/bpg_pkg/bpg_pkg_dsp.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "usf/bpg_rsp/bpg_rsp_manage.h"
#include "usf/bpg_rsp/bpg_rsp.h"
#include "bpg_rsp_internal_ops.h"

static int bpg_rsp_manage_load_commit_dsp(
    gd_app_context_t app, gd_app_module_t module, bpg_rsp_manage_t bpg_rsp_manage, cfg_t cfg)
{
    cfg_t commit_to;

    if ((commit_to = cfg_find_cfg(cfg, "commit-to"))) {
        bpg_pkg_dsp_t dsp = bpg_pkg_dsp_create(gd_app_alloc(app));
        if (dsp == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: create commit dsp fail",
                gd_app_module_name(module));
            return -1;
        }

        if (bpg_pkg_dsp_load(dsp, commit_to, gd_app_em(app)) != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: create commit dsp: load fail",
                gd_app_module_name(module));
            bpg_pkg_dsp_free(dsp);
            return -1;
        }

        bpg_rsp_manage_set_commit_dsp(bpg_rsp_manage, dsp);
    }

    return 0;
}

static int bpg_rsp_manage_load_queue_infos(
    gd_app_context_t app, gd_app_module_t module, bpg_rsp_manage_t bpg_rsp_manage, cfg_t cfg)
{
    struct cfg_it queue_it;
    cfg_t queue_cfg;

    cfg_it_init(&queue_it, cfg_find_cfg(cfg, "logic-queue"));

    while((queue_cfg = cfg_it_next(&queue_it))) {
        const char * name;
        const char * scope_name;
        bpg_rsp_queue_scope_t scope;
        struct bpg_rsp_queue_info * queue;

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
            scope = bpg_rsp_queue_scope_global;
        }
        else if (strcmp(scope_name, "client") == 0) {
            scope = bpg_rsp_queue_scope_client;
        }
        else {
            CPE_ERROR(
                gd_app_em(app), "%s: create: create logic queue %s: scope not configured!",
                gd_app_module_name(module), name);
            return -1;
        }

        queue = bpg_rsp_queue_info_create(
            bpg_rsp_manage, name, scope,
            cfg_get_uint32(queue_cfg, "max-count", 0));
        if (queue == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: create logic queue %s: create fail!",
                gd_app_module_name(module), name);
            return -1;
        }

        if (cfg_get_int32(queue_cfg, "is-default", 0)) {
            if (bpg_rsp_manage->m_default_queue_info != NULL) {
                CPE_ERROR(
                    gd_app_em(app), "%s: create: create logic queue %s: default queue info already exist, it is %s!",
                    gd_app_module_name(module), name, bpg_rsp_queue_name(bpg_rsp_manage->m_default_queue_info));
                return -1;
            }
            else {
                bpg_rsp_manage->m_default_queue_info = queue;
            }
        }

        if (bpg_rsp_manage->m_debug) {
            CPE_INFO(
                gd_app_em(app), "%s: create: create logic queue %s: scope=%s, max-count=%d!",
                gd_app_module_name(module), cpe_hs_data(queue->m_name), scope_name, queue->m_max_count);
        }
    }

    if (bpg_rsp_manage->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: default queue %s!",
            gd_app_module_name(module), 
            bpg_rsp_manage->m_default_queue_info
            ? cpe_hs_data(bpg_rsp_manage->m_default_queue_info->m_name)
            : "none");
    }

    return 0;
}

static int bpg_rsp_manage_load_forward_dsp(
    gd_app_context_t app, gd_app_module_t module, bpg_rsp_manage_t bpg_rsp_manage, cfg_t cfg)
{
    bpg_pkg_dsp_t forward_dsp;

    forward_dsp = bpg_pkg_dsp_create(gd_app_alloc(app));
    if (forward_dsp == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: create forward forward_dsp fail",
            gd_app_module_name(module));
        return -1;
    }

    if (bpg_pkg_dsp_load(forward_dsp, cfg_find_cfg(cfg, "forward-to"), gd_app_em(app)) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: create forward forward_dsp: load fail",
            gd_app_module_name(module));
        bpg_pkg_dsp_free(forward_dsp);
        return -1;
    }
    bpg_rsp_manage_set_forward_dsp(bpg_rsp_manage, forward_dsp);

    return 0;
}

EXPORT_DIRECTIVE
int bpg_rsp_manage_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    bpg_rsp_manage_t bpg_rsp_manage;
    bpg_rsp_manage_dp_scope_t scope;
    const char * str_scope;
    logic_manage_t logic_mgr;
    logic_executor_mgr_t executor_mgr;
    bpg_pkg_manage_t pkg_manage;
    cfg_t child_cfg;
    const char * executor_mgr_name;
    const char * pkg_manage_name;
    const char * load_from;
    const char * dsp_recv_at;

    str_scope = cfg_get_string(cfg, "scope", "global");
    if (strcmp(str_scope, "global") == 0) {
        scope = bpg_rsp_manage_dp_scope_global;
    }
    else if (strcmp(str_scope, "local") == 0) {
        scope = bpg_rsp_manage_dp_scope_local;
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

    pkg_manage_name = cfg_get_string(cfg, "pkg-manage", NULL);
    if (pkg_manage_name == NULL) {
        CPE_ERROR(
            gd_app_em(app),
            "%s: create: pkg-manage not configured",
            gd_app_module_name(module));
        return -1;
    }

    pkg_manage = bpg_pkg_manage_find_nc(app, pkg_manage_name);
    if (pkg_manage == NULL) {
        CPE_ERROR(
            gd_app_em(app),
            "%s: create: pkg-manage %s not exist",
            gd_app_module_name(module), pkg_manage_name);
        return -1;
    }

    bpg_rsp_manage = 
        bpg_rsp_manage_create(
            app,
            gd_app_module_name(module),
            scope,
            logic_mgr,
            executor_mgr,
            pkg_manage,
            NULL);
    if (bpg_rsp_manage == NULL) {
        return -1;
    }

    bpg_rsp_manage->m_debug = cfg_get_int32(cfg, "debug", 0);

    if ((dsp_recv_at = cfg_get_string(cfg, "recv-at", NULL))) {
        if (bpg_rsp_manage_set_dispatch_at(bpg_rsp_manage, dsp_recv_at) != 0) {
            bpg_rsp_manage_free(bpg_rsp_manage);
            return -1;
        }
    }

    if (bpg_rsp_manage_load_commit_dsp(app, module, bpg_rsp_manage, cfg) != 0) {
        bpg_rsp_manage_free(bpg_rsp_manage);
        return -1;
    }

    if (bpg_rsp_manage_load_forward_dsp(app, module, bpg_rsp_manage, cfg) != 0) {
        bpg_rsp_manage_free(bpg_rsp_manage);
        return -1;
    }

    if (bpg_rsp_manage_load_queue_infos(app, module, bpg_rsp_manage, cfg) != 0) {
        bpg_rsp_manage_free(bpg_rsp_manage);
        return -1;
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
                bpg_rsp_manage_free(bpg_rsp_manage);
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
                bpg_rsp_manage_free(bpg_rsp_manage);
                return -1;
            }
        }

        if (bpg_rsp_build(bpg_rsp_manage, child_cfg, rsps_metalib ? dr_store_lib(rsps_metalib) : NULL, gd_app_em(app)) != 0) {
            bpg_rsp_manage_free(bpg_rsp_manage);
            return -1;
        }
    }

    bpg_rsp_manage->m_rsp_max_size =
        cfg_get_uint32(cfg, "rsp-max-size", bpg_rsp_manage->m_rsp_max_size);

    if (bpg_rsp_manage->m_debug) {
        CPE_INFO(
            gd_app_em(app),
            "%s: create: done. rsp-max-size=%d",
            gd_app_module_name(module), (int)bpg_rsp_manage->m_rsp_max_size);
    }

    return 0;
}

EXPORT_DIRECTIVE
void bpg_rsp_manage_app_fini(gd_app_context_t app, gd_app_module_t module) {
    bpg_rsp_manage_t bpg_rsp_manage;

    bpg_rsp_manage = bpg_rsp_manage_find_nc(app, gd_app_module_name(module));
    if (bpg_rsp_manage) {
        bpg_rsp_manage_free(bpg_rsp_manage);
    }
}
