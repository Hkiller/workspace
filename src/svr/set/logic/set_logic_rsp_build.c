#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/buffer.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "usf/logic/logic_executor_ref.h"
#include "usf/logic/logic_executor_mgr.h"
#include "usf/logic/logic_executor_type.h"
#include "svr/set/logic/set_logic_rsp_manage.h"
#include "svr/set/logic/set_logic_rsp.h"
#include "set_logic_rsp_ops.h"

static int set_logic_rsp_str_cmd_cvt(int32_t * r, const char * str, void * ctx, error_monitor_t em) {
    int dr_result;

    if (ctx == NULL) return -1;

    if (dr_lib_find_macro_value(&dr_result, (LPDRMETALIB)ctx, str) == 0) {
        *r = dr_result;
        return 0;
    }
    else {
        return -1;
    } 
}

static int set_logic_rsp_create_dp_rsp_and_bind(set_logic_rsp_t set_logic_rsp, cfg_t cfg, LPDRMETALIB metalib, error_monitor_t em) {
    dp_rsp_t dp_rsp;
    cfg_t cfg_respons;

    cfg_respons = cfg_find_cfg(cfg, "respons-to");
    if (cfg_respons == NULL) return 0;

    dp_rsp = dp_rsp_create(
        set_logic_rsp->m_mgr->m_dp,
        set_logic_rsp_name(set_logic_rsp));
    if (dp_rsp == NULL) {
        CPE_ERROR(
            em,
            "%s: create rsp %s: create dp_rsp fail, maybe name duplicate!",
            set_logic_rsp_manage_name(set_logic_rsp->m_mgr), set_logic_rsp_name(set_logic_rsp));
        return -1;
    }

    dp_rsp_set_processor(dp_rsp, set_logic_rsp_execute, set_logic_rsp);

    if (dp_rsp_bind_by_cfg_ex(dp_rsp, cfg_respons, set_logic_rsp_str_cmd_cvt, metalib, em) != 0) {
        CPE_ERROR(
            em,
            "%s: create rsp %s: bind rsps by cfg fail!",
            set_logic_rsp_manage_name(set_logic_rsp->m_mgr), set_logic_rsp_name(set_logic_rsp));
        dp_rsp_free(dp_rsp);
        return -1;
    }
    else {
        return 0;
    }
}

static set_logic_rsp_t set_logic_rsp_build_one(set_logic_rsp_manage_t mgr, cfg_t cfg, LPDRMETALIB metalib, error_monitor_t em) {
    set_logic_rsp_t rsp;
    const char * name;
    cfg_t cfg_executor;
    const char * group_name;
    const char * queue_name;
    logic_executor_type_group_t type_group;
    logic_executor_ref_t executor_ref;

    assert(mgr);
    if (cfg == 0) return NULL;

    name = cfg_get_string(cfg, "name", NULL);
    if (name == NULL) {
        CPE_ERROR(mgr->m_em, "%s: create rsp: no name configured!", set_logic_rsp_manage_name(mgr)) ;
        return NULL;
    }
    rsp = set_logic_rsp_create(mgr, name);
    if (rsp == NULL) {
        CPE_ERROR(mgr->m_em, "%s: create rsp: create %s fail", set_logic_rsp_manage_name(mgr), name) ;
        return NULL;
    }

    group_name = cfg_get_string(cfg, "operations-from", NULL);
    type_group = logic_executor_type_group_find_nc(mgr->m_app, group_name);
    if (type_group == NULL) {
        CPE_ERROR(
            mgr->m_em, "%s: create rsp %s: executor_type_group '%s' not exist! (read from operations-from)",
            set_logic_rsp_manage_name(mgr), name, (group_name ? group_name : "default")) ;
        set_logic_rsp_free(rsp);
        return NULL;
    }

    cfg_executor = cfg_find_cfg(cfg, "operations");
    if (cfg_executor == NULL) {
        CPE_ERROR(mgr->m_em, "%s: create rsp %s: no executor configured! (read from operations)", set_logic_rsp_manage_name(mgr), name) ;
        set_logic_rsp_free(rsp);
        return NULL;
    }

    queue_name = cfg_get_string(cfg, "queue", NULL);
    if (queue_name == NULL) {
        queue_name = 
            mgr->m_default_queue_info
            ? set_logic_rsp_queue_name(mgr->m_default_queue_info)
            : NULL;
    }
    else if (strcasecmp(queue_name, "none") == 0) {
        queue_name = NULL;
    }

    if (set_logic_rsp_set_queue(rsp, queue_name) != 0) {
        set_logic_rsp_free(rsp);
        return NULL;
    }

    executor_ref =
        logic_executor_mgr_import(
            mgr->m_executor_mgr,
            name,
            mgr->m_logic_mgr,
            type_group,
            cfg_executor);
    if (executor_ref == NULL) {
        CPE_ERROR(mgr->m_em, "%s: create rsp %s: create executor fail!", set_logic_rsp_manage_name(mgr), name) ;
        set_logic_rsp_free(rsp);
        return NULL;
    }

    set_logic_rsp_set_executor(rsp, executor_ref);
    logic_executor_ref_dec(executor_ref);

    if (cfg_get_int32(cfg, "debug", 0)) {
        set_logic_rsp_flag_enable(rsp, set_logic_rsp_flag_debug);
    }

    if (cfg_get_int32(cfg, "append-info-manual", 0)) {
        set_logic_rsp_flag_enable(rsp, set_logic_rsp_flag_append_info_manual);
    }

    if (set_logic_rsp_create_dp_rsp_and_bind(rsp, cfg, metalib, em) != 0) {
        set_logic_rsp_free(rsp);
        return NULL;
    }

    return rsp;
}

static int set_logic_rsp_build_i(set_logic_rsp_manage_t mgr, cfg_t cfg, mem_buffer_t buf, cfg_t root, LPDRMETALIB metalib, error_monitor_t em) {
    struct cfg_it child_it;
    cfg_t child;
    int rv;

    rv = 0;

    if (cfg == NULL) {
        //do nothing!
    }
    else if (cfg_type(cfg) == CPE_CFG_TYPE_SEQUENCE) {
        cfg_it_init(&child_it, cfg);
        while((child = cfg_it_next(&child_it))) {
            set_logic_rsp_t rsp = set_logic_rsp_build_one(mgr, child, metalib, em);
            if (rsp == NULL) {
                CPE_ERROR(
                    em, "%s: %s: create fail",
                    set_logic_rsp_manage_name(mgr), cfg_path(buf, cfg, root));
                rv = -1;
            }
        }
    }
    else if (cfg_type(cfg) == CPE_CFG_TYPE_STRUCT) {
        cfg_it_init(&child_it, cfg);
        while((child = cfg_it_next(&child_it))) {
            if (set_logic_rsp_build_i(mgr, child, buf, root, metalib, em) != 0) {
                rv = -1;
            }
        } 
    }
    else {
        CPE_ERROR(
            em, "%s: %s: not support build rsp from type %d",
            set_logic_rsp_manage_name(mgr), cfg_path(buf, cfg, root), cfg_type(cfg));
        rv = -1;
    }

    return rv;
}

int set_logic_rsp_build(set_logic_rsp_manage_t mgr, cfg_t cfg, LPDRMETALIB metalib, error_monitor_t em) {
    struct mem_buffer buffer;
    int r;

    mem_buffer_init(&buffer, NULL);

    r = set_logic_rsp_build_i(mgr, cfg, &buffer, cfg, metalib, em);

    mem_buffer_clear(&buffer);

    return r;
}



