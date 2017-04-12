#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_manage.h"
#include "usf/logic/logic_context.h"
#include "usf/logic/logic_data.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_data.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "usf/bpg_pkg/bpg_pkg_dsp.h"
#include "usf/bpg_rsp/bpg_rsp_manage.h"
#include "usf/bpg_rsp/bpg_rsp_addition.h"
#include "usf/bpg_rsp/bpg_rsp.h"
#include "protocol/bpg_rsp/bpg_rsp_carry_info.h"
#include "bpg_rsp_internal_ops.h"

static int bpg_rsp_commit_build_pkg(bpg_rsp_t rsp, logic_context_t op_context, dp_req_t pkg, error_monitor_t em);
static int bpg_rsp_commit_build_carry_data(bpg_rsp_t rsp, logic_context_t op_context, dp_req_t pkg, error_monitor_t em);

void bpg_rsp_commit(logic_context_t op_context, void * user_data) {
    bpg_rsp_t bpg_rsp;
    bpg_rsp_manage_t bpg_mgr;
    dp_req_t response_buf;
    error_monitor_t em;
    logic_data_t bpg_private_data;
    BPG_CARRY_INFO * bpg_private;

    bpg_rsp = (bpg_rsp_t)user_data;
    assert(bpg_rsp);

    bpg_mgr = bpg_rsp->m_mgr;
    assert(bpg_mgr);

    em = bpg_mgr->m_em;

    if (bpg_mgr->m_commit_dsp == NULL) {
        CPE_ERROR(
            em, "%s.%s: bpg_rsp_commit: no commit-to configured, ignore commit!",
            bpg_rsp_manage_name(bpg_mgr), bpg_rsp_name(bpg_rsp));
        bpg_rsp_manage_free_context(bpg_mgr, op_context);
        return;
    }

    bpg_private_data = logic_context_data_find(op_context, "bpg_carry_info");
    if (bpg_private_data == NULL) {
        CPE_ERROR(
            em, "%s.%s: bpg_rsp_commit: no bpg_carry_info in context!",
            bpg_rsp_manage_name(bpg_mgr), bpg_rsp_name(bpg_rsp));
        bpg_rsp_manage_free_context(bpg_mgr, op_context);
        return;
    }

    bpg_private = (BPG_CARRY_INFO *)logic_data_data(bpg_private_data);
    if (bpg_private->no_response) {
        if (bpg_rsp_pkg_need_debug_detail(bpg_rsp, NULL)) {
            CPE_INFO(
                em, "%s.%s: bpg_rsp_commit: ignore send response!",
                bpg_rsp_manage_name(bpg_mgr), bpg_rsp_name(bpg_rsp));
        }
        bpg_rsp_manage_free_context(bpg_mgr, op_context);
        return;
    }

    response_buf = bpg_rsp_manage_rsp_buf(bpg_mgr);
    if (response_buf == NULL) {
        CPE_ERROR(em, "%s: bpg_rsp_commit: response buf is NULL!", bpg_rsp_manage_name(bpg_mgr));
        bpg_rsp_manage_free_context(bpg_mgr, op_context);
        return;
    }

    bpg_pkg_init(response_buf);
    bpg_pkg_set_sn(response_buf, bpg_private->sn);
    bpg_pkg_set_client_id(response_buf, bpg_private->clientId);
    bpg_pkg_set_errno(response_buf, logic_context_errno(op_context));
    bpg_pkg_set_cmd(response_buf, bpg_private->cmd);

    if (bpg_rsp_commit_build_carry_data(bpg_rsp, op_context, response_buf, em) != 0) {
        bpg_rsp_manage_free_context(bpg_mgr, op_context);
        return;
    }

    if (bpg_mgr->m_pkg_init) {
        if (bpg_mgr->m_pkg_init(op_context, response_buf, bpg_mgr->m_ctx_ctx) != 0) {
            CPE_ERROR(
                em, "%s.%s: bpg_rsp_commit: user pkg init fail!",
                bpg_rsp_manage_name(bpg_mgr), bpg_rsp_name(bpg_rsp));
            goto SEND_ERROR_RESPONSE;
        }
    }

    if (bpg_rsp_commit_build_pkg(bpg_rsp, op_context, response_buf, em) != 0) {
        goto SEND_ERROR_RESPONSE;
    }


    if (bpg_pkg_dsp_dispatch(bpg_mgr->m_commit_dsp, response_buf, em) != 0) {
        CPE_ERROR(em, "%s.%s: bpg_rsp_commit: dispatch fail!", bpg_rsp_manage_name(bpg_mgr), bpg_rsp_name(bpg_rsp));
        goto SEND_ERROR_RESPONSE;
    }

    bpg_rsp_manage_free_context(bpg_mgr, op_context);
    return;

SEND_ERROR_RESPONSE:
    bpg_pkg_init(response_buf);
    bpg_pkg_set_sn(response_buf, bpg_private->sn);
    bpg_pkg_set_client_id(response_buf, bpg_private->clientId);
    bpg_pkg_set_errno(response_buf, -1);
    bpg_pkg_set_cmd(response_buf, bpg_private->cmd);
    //TODO: bpg_pkg_set_connection_id(response_buf, bpg_private->connectionId);

    if (bpg_pkg_dsp_dispatch(bpg_mgr->m_commit_dsp, response_buf, em) != 0) {
        CPE_ERROR(em, "%s.%s: bpg_rsp_commit: send error response fail!", bpg_rsp_manage_name(bpg_mgr), bpg_rsp_name(bpg_rsp));
        bpg_rsp_manage_free_context(bpg_mgr, op_context);
        return;
    }

    bpg_rsp_manage_free_context(bpg_mgr, op_context);
    return;
}

static int bpg_rsp_commit_build_pkg_append_info_from_ctx(
    bpg_rsp_t rsp, logic_context_t op_context, dp_req_t pkg, LPDRMETA data_meta, error_monitor_t em)
{
    logic_data_t data;
    size_t size;

    if (bpg_rsp_flag_is_enable(rsp, bpg_rsp_flag_append_info_manual)) return 0;

    data = logic_context_data_find(op_context, dr_meta_name(data_meta));
    if (data == NULL) return -1;

    size = dr_meta_calc_data_len(data_meta, logic_data_data(data), logic_data_capacity(data));

    if (bpg_pkg_add_append_data(pkg, data_meta, logic_data_data(data), size, em) != 0) {
        CPE_ERROR(
            em, "%s.%s: copy_ctx_to_pdu: %s: append data to pkg fail!",
            bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp), dr_meta_name(data_meta));
        return 0;
    }

    if (bpg_rsp_pkg_need_debug_detail(rsp, pkg)) {
        CPE_INFO(
            em, "%s.%s: copy_ctx_to_pdu: %s: append data to pkg from context success, write-size="  FMT_SIZE_T "!",
            bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp), dr_meta_name(data_meta), size);
    }

    return 0;
}

static int bpg_rsp_commit_build_pkg_append_info_from_builder(
    bpg_rsp_t rsp, logic_context_t op_context, dp_req_t pkg,
    const char * data_name, error_monitor_t em)
{
    bpg_rsp_pkg_builder_t pkg_builder;

    TAILQ_FOREACH(pkg_builder, &rsp->m_mgr->m_pkg_builders, m_next) {
        if (pkg_builder->m_build_fun) {
            switch(pkg_builder->m_build_fun(pkg, op_context, data_name, pkg_builder->m_build_ctx)) {
            case bpg_pkg_build_result_success: {
                if (bpg_rsp_pkg_need_debug_detail(rsp, pkg)) {
                    CPE_INFO(
                        em, "%s.%s: copy_ctx_to_pdu: %s: append data to pkg from context success!",
                        bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp), data_name);
                }
                return 0;
            }
            case bpg_pkg_build_result_fail:
                return 0;
            case bpg_pkg_build_result_unknown:
                continue;
            }
        }
    }

    return -1;
}

static void bpg_rsp_commit_build_pkg_append_info_for_addition(
    bpg_rsp_t rsp, logic_context_t op_context, dp_req_t pkg, LPDRMETALIB metalib, error_monitor_t em)
{
    int i, addition_count;

    addition_count = bpg_rsp_addition_data_count(op_context);

    for (i = 0; i < addition_count; ++i) {
        uint32_t meta_id = bpg_rsp_addition_data_at(op_context, i);
        LPDRMETA meta = dr_lib_find_meta_by_id(metalib, meta_id);
        if (meta == NULL) {
            CPE_ERROR(
                em, "%s.%s: copy_ctx_to_pdu: for addition: meta with id %d not exist in lib \"%s\"!",
                bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp), meta_id, dr_lib_name(metalib));
            continue;
        }

        if (bpg_rsp_commit_build_pkg_append_info_from_ctx(rsp, op_context, pkg, meta, em) == 0) {
            continue;
        }

        if (bpg_rsp_commit_build_pkg_append_info_from_builder(rsp, op_context, pkg, dr_meta_name(meta), em) == 0) {
            continue;
        }

        if (bpg_rsp_pkg_need_debug_detail(rsp, pkg)) {
            CPE_INFO(
                em, "%s.%s: copy_ctx_to_pdu: for addition: %s: can`t find data!",
                bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp), dr_meta_name(meta));
        }
    }
}

static void bpg_rsp_commit_build_pkg_append_info_for_copy_info(
    bpg_rsp_t rsp, logic_context_t op_context, dp_req_t pkg, LPDRMETALIB metalib, error_monitor_t em)
{
    struct bpg_rsp_copy_info * copy_info;


    TAILQ_FOREACH(copy_info, &rsp->m_ctx_to_pdu, m_next) {
        const char * copy_data_name;
        LPDRMETA data_meta;

        copy_data_name = bpg_rsp_copy_info_data(copy_info);

        data_meta = dr_lib_find_meta_by_name(metalib, copy_data_name);
        if (data_meta == NULL) {
            CPE_ERROR(
                em, "%s.%s: copy_ctx_to_pdu: %s: meta not exist!",
                bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp), copy_data_name);
            continue;
        }

        if (bpg_rsp_addition_data_exist(op_context, dr_meta_id(data_meta))) continue;

        if (bpg_rsp_commit_build_pkg_append_info_from_ctx(rsp, op_context, pkg, data_meta, em) == 0) {
            continue;
        }

        if (bpg_rsp_commit_build_pkg_append_info_from_builder(rsp, op_context, pkg, copy_data_name, em) == 0) {
            continue;
        }

        CPE_ERROR(
            em, "%s.%s: copy_ctx_to_pdu: %s: can`t find data!",
            bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp), copy_data_name);
    }
}

static int bpg_rsp_commit_build_pkg_append_info(bpg_rsp_t rsp, logic_context_t op_context, dp_req_t pkg, error_monitor_t em) {
    LPDRMETALIB metalib = bpg_pkg_manage_data_metalib(rsp->m_mgr->m_pkg_manage);
    if (metalib == NULL) {
        CPE_ERROR(
            em, "%s.%s: copy_ctx_to_pdu: no metalib!",
            bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp));
        return -1;
    }

    bpg_rsp_commit_build_pkg_append_info_for_addition(rsp, op_context, pkg, metalib, em);

    if (logic_context_errno(op_context) == 0) {
        bpg_rsp_commit_build_pkg_append_info_for_copy_info(rsp, op_context, pkg, metalib, em);
    }

    return 0;
}

static int bpg_rsp_commit_build_pkg_main_info(bpg_rsp_t rsp, logic_context_t op_context, dp_req_t pkg, error_monitor_t em) {
    logic_data_t data;
    LPDRMETA meta;
    size_t size;

    meta = bpg_pkg_main_data_meta(pkg, NULL);
    if (meta == NULL) return 0;

    data = logic_context_data_find(op_context, dr_meta_name(meta));
    if (data == NULL) {
        CPE_ERROR(
            em, "%s.%s: copy_ctx_to_pdu: main: can`t find %s from ctx!",
            bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp), dr_meta_name(meta));
        return 0;
    }

    size = dr_meta_calc_data_len(meta, logic_data_data(data), logic_data_capacity(data));
    if (bpg_pkg_set_main_data(pkg, logic_data_data(data), size, em) != 0) {
        CPE_ERROR(
            em, "%s.%s: copy_ctx_to_pdu: main: set data fail, meta = %s!",
            bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp), dr_meta_name(meta));
        return -1;
    }

    if (bpg_rsp_pkg_need_debug_detail(rsp, pkg)) {
        CPE_INFO(
            em, "%s.%s: copy_ctx_to_pdu: main: meta = %s, wirte-size=%d!",
            bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp), dr_meta_name(meta), (int)size);
    }
            
    return 0;
}

int bpg_rsp_commit_build_pkg(bpg_rsp_t rsp, logic_context_t op_context, dp_req_t pkg, error_monitor_t em) {
    if (bpg_rsp_commit_build_pkg_main_info(rsp, op_context, pkg, em) != 0) return -1;
    if (bpg_rsp_commit_build_pkg_append_info(rsp, op_context, pkg, em) != 0) return -1;
    return 0;
}

void bpg_rsp_manage_free_context(bpg_rsp_manage_t mgr, logic_context_t op_context) {
    if (mgr->m_ctx_fini) {
        mgr->m_ctx_fini(op_context, mgr->m_ctx_ctx);
    }
    logic_context_free(op_context);
}

static int bpg_rsp_commit_build_carry_data(bpg_rsp_t rsp, logic_context_t op_context, dp_req_t pkg, error_monitor_t em) {
    logic_data_t carry_info_list_data;
    BPG_CARRY_METAS * carry_info_list;
    uint32_t i;
    struct dp_req_it child_it;
    dp_req_t child_data;

    carry_info_list_data = logic_context_data_find(op_context, "bpg_carry_metas");
    if (carry_info_list_data == NULL) return 0;

    carry_info_list = logic_data_data(carry_info_list_data);
    dp_req_childs(pkg, &child_it);

    for(i = 0, child_data = dp_req_next(&child_it); i < carry_info_list->count; ++i, child_data = dp_req_next(&child_it)) {
        logic_data_t carry_data = logic_context_data_find(op_context, carry_info_list->data[i].name);

        if (carry_data == NULL) {
            CPE_ERROR(
                em, "%s.%s: copy_carry data %s: data not exist!",
                bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp), carry_info_list->data[i].name);
            return -1;
        }

        if (child_data && dp_req_is_type(child_data, req_type_bpg_pkg)) {
            child_data = dp_req_next(&child_it);
        }

        if (child_data == NULL) {
            child_data = dp_req_create(dp_req_mgr(pkg), 0);
            if (child_data == NULL) {
                CPE_ERROR(
                    em, "%s.%s: copy_carry data %s: create child_data fial!",
                    bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp), carry_info_list->data[i].name);
                return -1;
            }
            else {
                dp_req_add_to_parent(child_data, pkg);
            }
        }

        dp_req_set_buf(child_data, logic_data_data(carry_data), logic_data_capacity(carry_data));
        dp_req_set_size(child_data, logic_data_capacity(carry_data));
        dp_req_set_meta(child_data, logic_data_meta(carry_data));
    }

    while(child_data) {
        dp_req_t next = dp_req_next(&child_it);

        if (!dp_req_is_type(child_data, req_type_bpg_pkg)) {
            if (dp_req_manage_by_parent(child_data)) {
                dp_req_free(child_data);
            }
            else {
                dp_req_set_parent(child_data, NULL);
            }
        }

        child_data = next;
    }

    return 0;
}
