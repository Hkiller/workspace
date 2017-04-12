#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/dp/dp_binding.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_manage.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "usf/logic/logic_context.h"
#include "usf/logic/logic_queue.h"
#include "usf/logic/logic_executor_ref.h"
#include "usf/logic/logic_data.h"
#include "usf/bpg_pkg/bpg_pkg_data.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "usf/bpg_rsp/bpg_rsp_manage.h"
#include "usf/bpg_rsp/bpg_rsp.h"
#include "usf/bpg_rsp/bpg_rsp_carry_info.h"
#include "protocol/bpg_rsp/bpg_rsp_carry_info.h"
#include "bpg_rsp_internal_ops.h"

extern char g_metalib_carry_package[];

enum bpg_rsp_queue_next_op {
    bpg_rsp_queue_next_op_success
    , bpg_rsp_queue_next_op_exec
    , bpg_rsp_queue_next_op_error
};

static int bpg_rsp_copy_pkg_to_ctx(bpg_rsp_t rsp, logic_context_t op_context, dp_req_t req, error_monitor_t em);
static void bpg_rsp_commit_error(bpg_rsp_t rsp, logic_context_t op_context, int err);
static enum bpg_rsp_queue_next_op bpg_rsp_queue_context(bpg_rsp_manage_t bpg_mgr, bpg_rsp_t rsp, logic_context_t op_context, uint64_t client_id, error_monitor_t em);
static int bpg_rsp_copy_carry_metas(bpg_rsp_manage_t bpg_mgr, const char * rsp_name, logic_context_t from_context, logic_context_t to_context, error_monitor_t em);

int bpg_rsp_execute(dp_req_t dp_req, void * ctx, error_monitor_t em) {
    bpg_rsp_t bpg_rsp;
    bpg_rsp_manage_t bpg_mgr;
    logic_context_t op_context;
    enum bpg_rsp_queue_next_op next_op;

    bpg_rsp = (bpg_rsp_t)ctx;
    assert(bpg_rsp);

    bpg_mgr = bpg_rsp->m_mgr;
    assert(bpg_mgr);

    op_context = bpg_rsp_manage_create_context(bpg_mgr, dp_req, em);
    if (op_context == NULL) {
        CPE_ERROR(
            em, "%s.%s: bpg_rsp_execute: create op_context for pkg fail!",
            bpg_rsp_manage_name(bpg_mgr), bpg_rsp_name(bpg_rsp));
        return 0;
    }

    if (bpg_rsp_flag_is_enable(bpg_rsp, bpg_rsp_flag_debug)) {
        logic_context_flag_enable(op_context, logic_context_flag_debug);
    }

    /*aft user init, we should commit any error*/
    if (bpg_rsp_copy_pkg_to_ctx(bpg_rsp, op_context, dp_req, em) != 0) {
        CPE_ERROR(
            em, "%s.%s: bpg_rsp_execute: copy data from pdu to context!",
            bpg_rsp_manage_name(bpg_mgr), bpg_rsp_name(bpg_rsp));
        bpg_rsp_commit_error(bpg_rsp, op_context, -1);
        return 0;
    }

    if (logic_context_bind(
            op_context,
            logic_executor_ref_executor(bpg_rsp->m_executor_ref)) != 0)
    {
        CPE_ERROR(
            em, "%s.%s: bpg_rsp_execute: bind executor to context fail!",
            bpg_rsp_manage_name(bpg_mgr), bpg_rsp_name(bpg_rsp));
        bpg_rsp_commit_error(bpg_rsp, op_context, -1);
        return 0;
    }

    next_op = bpg_rsp_queue_context(bpg_mgr, bpg_rsp, op_context, bpg_pkg_client_id(dp_req), em);
    switch(next_op) {
    case bpg_rsp_queue_next_op_success:
        logic_context_set_commit(op_context, bpg_rsp_commit, bpg_rsp);
        break;
    case bpg_rsp_queue_next_op_exec:
        logic_context_set_commit(op_context, bpg_rsp_commit, bpg_rsp);
        logic_context_execute(op_context);
        break;
    case bpg_rsp_queue_next_op_error:
        bpg_rsp_commit_error(bpg_rsp, op_context, -1);
        break;
    default:
        CPE_ERROR(
            em, "%s.%s: bpg_rsp_execute: unknown next op %d!",
            bpg_rsp_manage_name(bpg_mgr), bpg_rsp_name(bpg_rsp), next_op);
        bpg_rsp_commit_error(bpg_rsp, op_context, -1);
        return 0;
    }

    return 0;
}

static logic_queue_t
bpg_rsp_queue_get_or_create(
    bpg_rsp_manage_t mgr, bpg_rsp_t rsp, 
    cpe_hash_string_t queue_name, struct bpg_rsp_queue_info * queue_info, error_monitor_t em)
{
    logic_queue_t queue = logic_queue_find(mgr->m_logic_mgr, queue_name);
    if (queue) return queue;

    queue = logic_queue_create(mgr->m_logic_mgr, cpe_hs_data(queue_name));
    if (queue == NULL) {
        CPE_ERROR(
            em, "%s.%s: bpg_rsp_execute: create queue(%s) fail!",
            bpg_rsp_manage_name(mgr), bpg_rsp_name(rsp), cpe_hs_data(queue_name));
        return NULL;
    }

    logic_queue_set_max_count(queue, queue_info->m_max_count);

    if (mgr->m_debug) {
        CPE_INFO(
            em, "%s.%s: bpg_rsp_execute: create queue(%s) success, max-size=%d, size=%d!",
            bpg_rsp_manage_name(mgr), bpg_rsp_name(rsp),
            logic_queue_name(queue), logic_queue_max_count(queue), logic_queue_count(queue));
    }

    return queue;
}

static enum bpg_rsp_queue_next_op bpg_rsp_queue_context(
    bpg_rsp_manage_t mgr, bpg_rsp_t rsp, logic_context_t op_context, uint64_t client_id, error_monitor_t em)
{
    struct bpg_rsp_queue_info * queue_info = rsp->m_queue_info;
    logic_queue_t queue;

    if (queue_info == NULL) return bpg_rsp_queue_next_op_exec;

    switch(queue_info->m_scope) {
    case bpg_rsp_queue_scope_global:
        queue = bpg_rsp_queue_get_or_create(mgr, rsp, queue_info->m_name, queue_info, em);
        break;
    case bpg_rsp_queue_scope_client: {
        cpe_hs_printf(
            (cpe_hash_string_t)queue_info->m_name_buf,
            sizeof(queue_info->m_name_buf),
            "%s."FMT_SIZE_T, bpg_rsp_queue_name(queue_info), client_id);
        queue = bpg_rsp_queue_get_or_create(mgr, rsp, (cpe_hash_string_t)queue_info->m_name_buf, queue_info, em);
        break;
    }
    default:
        CPE_ERROR(
            em, "%s.%s: bpg_rsp_execute: bpg_rsp_queue_context: unknown scope type!",
            bpg_rsp_manage_name(mgr), bpg_rsp_name(rsp));
        return bpg_rsp_queue_next_op_error;
    }

    if (queue) {
        logic_queue_enqueue_tail(queue, op_context);
        if (mgr->m_debug) {
            CPE_INFO(
                em, "%s.%s: bpg_rsp_execute: add to queue(%s) tail, max-size=%d, size=%d!",
                bpg_rsp_manage_name(mgr), bpg_rsp_name(rsp),
                logic_queue_name(queue), logic_queue_max_count(queue), logic_queue_count(queue));
        }

        if (logic_queue_head(queue) == op_context) {
            return bpg_rsp_queue_next_op_exec;
        }
        else {
            return bpg_rsp_queue_next_op_success;
        }
    }
    else {
        CPE_ERROR(
            em, "%s.%s: bpg_rsp_execute: add to queue fail!",
            bpg_rsp_manage_name(mgr), bpg_rsp_name(rsp));
        return bpg_rsp_queue_next_op_error;
    }
}

static int bpg_rsp_copy_main_to_ctx(bpg_rsp_t rsp, logic_context_t op_context, dp_req_t req, error_monitor_t em) {
    LPDRMETA data_meta;
    logic_data_t data;
    bpg_rsp_manage_t mgr;
    size_t size;
    size_t meta_size;

    mgr = rsp->m_mgr;

    data_meta = bpg_pkg_main_data_meta(req, NULL);
    if (data_meta == NULL) {
        if (rsp->m_mgr->m_debug >= 2) {
            CPE_INFO(
                em, "%s.%s: bpg_rsp_execute: copy_pkg_to_ctx: get data meta fail!",
                bpg_rsp_manage_name(mgr), bpg_rsp_name(rsp));
        }
        return 0;
    }

    size = bpg_pkg_main_data_len(req);
    meta_size = dr_meta_size(data_meta);
    data = logic_context_data_get_or_create(op_context, data_meta, size < meta_size ? meta_size : size);
    if (data == NULL) {
        CPE_ERROR(
            em, "%s.%s: bpg_rsp_execute: copy_pkg_to_ctx: %s create data fail, capacity=%d!",
            bpg_rsp_manage_name(mgr), bpg_rsp_name(rsp), dr_meta_name(data_meta), (int)size);
        return -1;
    }

    memcpy(logic_data_data(data), bpg_pkg_main_data(req), size);

    return 0;
}

static int bpg_rsp_copy_append_to_ctx(bpg_rsp_t rsp, logic_context_t op_context, dp_req_t req, error_monitor_t em) {
    LPDRMETA data_meta;
    logic_data_t data;
    int i;
    bpg_rsp_manage_t mgr;
    int32_t append_info_count;
    size_t size;
    size_t meta_size;

    mgr = rsp->m_mgr;
    assert(mgr);

    append_info_count = bpg_pkg_append_info_count(req);
    for(i = 0; i < append_info_count; ++i) {
        bpg_pkg_append_info_t append_info;

        append_info = bpg_pkg_append_info_at(req, i);
        if (append_info == NULL) {
            CPE_ERROR(
                em, "%s.%s: bpg_rsp_execute: copy_pkg_to_ctx: append %d: append info is NULL!",
                bpg_rsp_manage_name(mgr), bpg_rsp_name(rsp), i);
            continue;
        }

        data_meta = bpg_pkg_append_data_meta(req, append_info, em);
        if (data_meta == NULL) {
            CPE_ERROR(
                em, "%s.%s: bpg_rsp_execute: copy_pkg_to_ctx: append %d: append meta not exist!",
                bpg_rsp_manage_name(mgr), bpg_rsp_name(rsp), i);
            return -1;
        }

        size = bpg_pkg_append_info_size(append_info);
        meta_size = dr_meta_size(data_meta);
        data = logic_context_data_get_or_create(op_context, data_meta, size < meta_size ? meta_size : size);
        if (data == NULL) {
            CPE_ERROR(
                em, "%s.%s: bpg_rsp_execute: copy_pkg_to_ctx: append %d: %s create data fail, capacity=%d!",
                bpg_rsp_manage_name(mgr), bpg_rsp_name(rsp), i, dr_meta_name(data_meta), (int)size);
            return -1;
        }

        memcpy(logic_data_data(data), bpg_pkg_append_data(req, append_info), size);
    }

    return 0;
}

int bpg_rsp_copy_pkg_to_ctx(bpg_rsp_t rsp, logic_context_t op_context, dp_req_t req, error_monitor_t em) {
    bpg_rsp_manage_t mgr;

    mgr = rsp->m_mgr;

    if (bpg_rsp_copy_main_to_ctx(rsp, op_context, req, em) != 0) return -1;
    if (bpg_rsp_copy_append_to_ctx(rsp, op_context, req, em) != 0) return -1;

    (void)mgr;
    return 0;
}

int bpg_rsp_copy_req_carry_data_to_ctx(bpg_rsp_manage_t mgr, logic_context_t op_context, dp_req_t bpg_req, error_monitor_t em) {
    struct dp_req_it childs;
    LPDRMETA carry_meta;
    logic_data_t data;
    dp_req_t child;
    int carry_info_count;
    LPDRMETA carry_info_list_meta;
    logic_data_t carry_info_list_data;
    BPG_CARRY_METAS * carry_info_list;
    size_t size;
    size_t meta_size;

    dp_req_childs(bpg_req, &childs);
    carry_info_count = 0;
    while((child = dp_req_next(&childs))) {
        carry_meta = dp_req_meta(child);
        if (carry_meta == NULL) continue;

        ++carry_info_count;
    }

    if (carry_info_count == 0) return 0;

    carry_info_list_meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_carry_package, "bpg_carry_metas");
    if (carry_info_list_meta == NULL) {
        CPE_ERROR(
            em, "%s: copy_bpg_carry_data: bpg_carry_metas meta not exist!",
            bpg_rsp_manage_name(mgr));
        return -1;
    }

    size = dr_meta_calc_dyn_size(carry_info_list_meta, carry_info_count);
    meta_size = dr_meta_size(carry_info_list_meta);

    carry_info_list_data =
        logic_context_data_get_or_create(op_context, carry_info_list_meta, size < meta_size ? meta_size : size);
    if (carry_info_list_data == NULL) {
        CPE_ERROR(
            em, "%s: copy_bpg_carry_data: create carry_info_list_data fail!",
            bpg_rsp_manage_name(mgr));
        return -1;
    }

    carry_info_list = logic_data_data(carry_info_list_data);
    carry_info_list->count = 0;

    dp_req_childs(bpg_req, &childs);
    while((child = dp_req_next(&childs))) {
        BPG_CARRY_META * carry_meta_info;
        size_t carry_size;
        size_t carry_meta_size;

        carry_meta = dp_req_meta(child);
        if (carry_meta == NULL) continue;

        carry_size = dp_req_size(child);
        carry_meta_size = dr_meta_size(carry_meta);

        data = logic_context_data_get_or_create(op_context, carry_meta, carry_size < carry_meta_size ? carry_meta_size : carry_size);
        if (data == NULL) {
            CPE_ERROR(
                em, "%s: copy_req_carry_data: %s create data fail, size=%d!",
                bpg_rsp_manage_name(mgr),
                dr_meta_name(carry_meta), (int)dp_req_size(child));
            return -1;
        }

        memcpy(logic_data_data(data), dp_req_data(child), dp_req_size(child));

        carry_meta_info = &carry_info_list->data[carry_info_list->count++];

        cpe_str_dup(carry_meta_info->name, sizeof(carry_meta_info->name), dr_meta_name(carry_meta));
    }

    return 0;
}

int bpg_rsp_copy_bpg_carry_data_to_ctx(bpg_rsp_manage_t mgr, logic_context_t op_context, dp_req_t bpg_req, error_monitor_t em) {
    LPDRMETA bpg_carry_data_meta;
    logic_data_t data;
    BPG_CARRY_INFO * buf;

    bpg_carry_data_meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_carry_package, "bpg_carry_info");
    if (bpg_carry_data_meta == NULL) {
        CPE_ERROR(
            em, "%s: copy_bpg_carry_data: bpg_carry_info meta not exist!",
            bpg_rsp_manage_name(mgr));
        return -1;
    }

    data = logic_context_data_get_or_create(op_context, bpg_carry_data_meta, dr_meta_size(bpg_carry_data_meta));
    if (data == NULL) {
        CPE_ERROR(
            em, "%s: copy_bpg_carry_data: %s create data fail, size=%d!",
            bpg_rsp_manage_name(mgr),
            dr_meta_name(bpg_carry_data_meta), (int)dr_meta_size(bpg_carry_data_meta));
        return -1;
    }

    buf = (BPG_CARRY_INFO *)logic_data_data(data);

    if (bpg_req) {
        buf->clientId = bpg_pkg_client_id(bpg_req);
        buf->sn = bpg_pkg_sn(bpg_req);
        buf->cmd = bpg_pkg_cmd(bpg_req);
        buf->no_response = bpg_pkg_flag_enable(bpg_req, bpg_pkg_flag_oneway) ? 1 : 0;
    }
    else {
        bzero(buf, sizeof(*buf));
    }

    return 0;
}

static void bpg_rsp_commit_error(bpg_rsp_t rsp, logic_context_t op_context, int err) {
    if (rsp->m_mgr->m_debug) {
        CPE_INFO(
            gd_app_em(rsp->m_mgr->m_app), "%s.%s: commit error, error=%d!",
            bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp), err);
    }

    logic_context_errno_set(op_context, err);
    bpg_rsp_commit(op_context, rsp);
}

logic_context_t bpg_rsp_manage_create_context(bpg_rsp_manage_t bpg_mgr, dp_req_t req, error_monitor_t em) {
    logic_context_t op_context;

    if (req == NULL) {
        op_context = logic_context_create(
            bpg_mgr->m_logic_mgr,
            INVALID_LOGIC_CONTEXT_ID,
            bpg_mgr->m_ctx_capacity);
    }
    else {
        uint32_t sn =
            bpg_rsp_manage_flag_is_enable(bpg_mgr, bpg_rsp_manage_flag_sn_use_client)
            ? bpg_pkg_sn(req)
            : INVALID_LOGIC_CONTEXT_ID;

        op_context = logic_context_create(bpg_mgr->m_logic_mgr, sn, bpg_mgr->m_ctx_capacity);
        if (op_context == NULL) {
            CPE_ERROR(
                em, "%s: create context: fail, capacity is %d!",
                bpg_rsp_manage_name(bpg_mgr), (int)bpg_mgr->m_ctx_capacity);
            return NULL;
        }

        if (req && bpg_rsp_copy_bpg_carry_data_to_ctx(bpg_mgr, op_context, req, em) != 0) {
            logic_context_free(op_context);
            return NULL;
        }

        if (bpg_rsp_copy_req_carry_data_to_ctx(bpg_mgr, op_context, req, em) != 0) {
            logic_context_free(op_context);
            return NULL;
        }
    }

    if (bpg_mgr->m_debug >= 2 || (req && bpg_pkg_debug_level(req) >= bpg_pkg_debug_progress)) {
        logic_context_flag_enable(op_context, logic_context_flag_debug);
    }

    if (bpg_mgr->m_ctx_init) {
        if (bpg_mgr->m_ctx_init(op_context, req, bpg_mgr->m_ctx_ctx) != 0) {
            CPE_ERROR(
                em, "%s: create context: use-ctx-init: init fail!",
                bpg_rsp_manage_name(bpg_mgr));
            logic_context_free(op_context);
            return NULL;
        }
    }
    
    return op_context;
}

logic_context_t
bpg_rsp_manage_create_follow_op_by_name(bpg_rsp_manage_t bpg_mgr, logic_context_t context, const char * rsp_name, error_monitor_t em) {
    bpg_rsp_t rsp;
    dp_rsp_t dp_rsp;
    logic_context_t follow_context;
    logic_data_t input_carry_data;
    logic_data_t carry_data;
    bpg_rsp_carry_info_t carry_info;
    logic_data_t carry_info_list_data;

    assert(rsp_name);

    rsp = bpg_rsp_find(bpg_mgr, rsp_name);
    if (rsp == NULL) {
        CPE_ERROR(
            em, "%s.%s: create follow op: fail, rsp not exist!",
            bpg_rsp_manage_name(bpg_mgr), rsp_name);
        return NULL;
    }

    follow_context =
        logic_context_create(
            bpg_mgr->m_logic_mgr,
            INVALID_LOGIC_CONTEXT_ID,
            bpg_mgr->m_ctx_capacity);
    if (follow_context == NULL) {
        CPE_ERROR(
            em, "%s.%s: create follow op: fail, capacity is %d!",
            bpg_rsp_manage_name(bpg_mgr), rsp_name, (int)bpg_mgr->m_ctx_capacity);
        return NULL;
    }

    if ((carry_info_list_data = logic_context_data_find(context, "bpg_carry_metas"))) {
        BPG_CARRY_METAS const * carry_info_list = logic_data_data(carry_info_list_data);
        uint32_t i;

        if (logic_context_data_copy(follow_context, carry_info_list_data) == NULL) {
            CPE_ERROR(
                em, "%s.%s: create follow op: bpg_carry_metas: data create fail!",
                bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp));
            logic_context_free(follow_context);
            return NULL;
        }

        for(i = 0; i < carry_info_list->count; ++i) {
            logic_data_t carry_data = logic_context_data_find(context, carry_info_list->data[i].name);

            if (carry_data == NULL) {
                CPE_ERROR(
                    em, "%s.%s: create follow op: %s: data not exist!",
                    bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp), carry_info_list->data[i].name);
                logic_context_free(follow_context);
                return NULL;
            }

            if (logic_context_data_copy(follow_context, carry_data) == NULL) {
                CPE_ERROR(
                    em, "%s.%s: create follow op: %s: copy data fail, size=%d!",
                    bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp), carry_info_list->data[i].name, (int)logic_data_capacity(carry_data));
                logic_context_free(follow_context);
                return NULL;
            }
        }
    }

    if ((input_carry_data = logic_context_data_find(context, "bpg_carry_info"))) {
        carry_data = logic_context_data_copy(follow_context, input_carry_data);
    }
    else {
        CPE_ERROR(
            em, "%s.%s: create follow op: bpg_carry_info not exist in input context!",
            bpg_rsp_manage_name(bpg_mgr), rsp_name);
        logic_context_free(follow_context);
        return NULL;
    }

    carry_info = (bpg_rsp_carry_info_t)logic_data_data(carry_data);
    assert(carry_info);

    if ((dp_rsp = bpg_rsp_dp(rsp))) {
        struct dp_binding_it binding_it;
        dp_binding_t only_binding;

        dp_rsp_bindings(&binding_it, dp_rsp);
        only_binding = dp_binding_next(&binding_it);
        if (only_binding && dp_binding_next(&binding_it) == NULL) {
            uint32_t cmd;
            if (dp_binding_numeric(&cmd, only_binding) == 0) {
                assert(carry_data != NULL);
                bpg_rsp_context_set_cmd(carry_info, cmd);
            }
        }
        else {
            bpg_rsp_context_set_no_response(carry_info, 1);
        }
    }
    else {
        bpg_rsp_context_set_no_response(carry_info, 1);
    }

    if (bpg_mgr->m_debug >= 2 || bpg_rsp_flag_is_enable(rsp, bpg_rsp_flag_debug)) {
        logic_context_flag_enable(follow_context, logic_context_flag_debug);
    }

    if (bpg_mgr->m_ctx_init) {
        if (bpg_mgr->m_ctx_init(follow_context, NULL, bpg_mgr->m_ctx_ctx) != 0) {
            CPE_ERROR(
                em, "%s.%s: create follow op: use-ctx-init: init fail!",
                bpg_rsp_manage_name(bpg_mgr), rsp_name);
            logic_context_free(follow_context);
            return NULL;
        }
    }

    if (logic_context_bind(
            follow_context,
            logic_executor_ref_executor(rsp->m_executor_ref)) != 0)
    {
        CPE_ERROR(
            em, "%s.%s: create follow op: bind executor to context fail!",
            bpg_rsp_manage_name(bpg_mgr), rsp_name);
        bpg_rsp_manage_free_context(bpg_mgr, follow_context);
        return NULL;
    }

    if (logic_context_queue(context)) {
        if (logic_queue_enqueue_after(context, follow_context) != 0) {
            CPE_ERROR(
                em, "%s.%s: create follow op: enqueue after input fail!",
                bpg_rsp_manage_name(bpg_mgr), rsp_name);
            bpg_rsp_manage_free_context(bpg_mgr, follow_context);
            return NULL;
        }
    }

    logic_context_set_commit(follow_context, bpg_rsp_commit, rsp);

    return follow_context;
}

logic_context_t
bpg_rsp_manage_create_op_by_name(
    bpg_rsp_manage_t bpg_mgr,
    logic_context_t from_context, 
    const char * rsp_name,
    error_monitor_t em)
{
    bpg_rsp_t rsp;
    dp_rsp_t dp_rsp;
    logic_context_t context;
    logic_data_t input_carry_data;
    logic_data_t carry_data;
    bpg_rsp_carry_info_t carry_info;
    enum bpg_rsp_queue_next_op next_op;

    assert(rsp_name);

    rsp = bpg_rsp_find(bpg_mgr, rsp_name);
    if (rsp == NULL) {
        CPE_ERROR(
            em, "%s.%s: create op: fail, rsp not exist!",
            bpg_rsp_manage_name(bpg_mgr), rsp_name);
        return NULL;
    }

    context =
        logic_context_create(
            bpg_mgr->m_logic_mgr,
            INVALID_LOGIC_CONTEXT_ID,
            bpg_mgr->m_ctx_capacity);
    if (context == NULL) {
        CPE_ERROR(
            em, "%s.%s: create op: fail, capacity is %d!",
            bpg_rsp_manage_name(bpg_mgr), rsp_name, (int)bpg_mgr->m_ctx_capacity);
        return NULL;
    }

    if ((input_carry_data = 
         from_context
         ? logic_context_data_find(from_context, "bpg_carry_info")
         : NULL))
    {
        carry_data = logic_context_data_copy(context, input_carry_data);
    }
    else {
        if (bpg_rsp_copy_bpg_carry_data_to_ctx(bpg_mgr, context, NULL, em) != 0) {
            CPE_ERROR(
                em, "%s.%s: create op: fail, create carry info fail!",
                bpg_rsp_manage_name(bpg_mgr), rsp_name);
            logic_context_free(context);
            return NULL;
        }
    }

    if (from_context) {
        if (bpg_rsp_copy_carry_metas(bpg_mgr, rsp_name, from_context, context, em) != 0) {
            CPE_ERROR(
                em, "%s.%s: create op: fail, create carry metas fail!",
                bpg_rsp_manage_name(bpg_mgr), rsp_name);
            logic_context_free(context);
            return NULL;
        }
    }

    carry_data = logic_context_data_find(context, "bpg_carry_info");
    assert(carry_data);

    carry_info = (bpg_rsp_carry_info_t)logic_data_data(carry_data);
    assert(carry_info);

    if ((dp_rsp = bpg_rsp_dp(rsp))) {
        struct dp_binding_it binding_it;
        dp_binding_t only_binding;

        dp_rsp_bindings(&binding_it, dp_rsp);
        only_binding = dp_binding_next(&binding_it);
        if (only_binding && dp_binding_next(&binding_it) == NULL) {
            uint32_t cmd;
            if (dp_binding_numeric(&cmd, only_binding) == 0) {
                bpg_rsp_context_set_cmd(carry_info, cmd);
            }
        }
    }
    else {
        bpg_rsp_context_set_no_response(carry_info, 1);
    }

    if (bpg_mgr->m_debug >= 2 || bpg_rsp_flag_is_enable(rsp, bpg_rsp_flag_debug)) {
        logic_context_flag_enable(context, logic_context_flag_debug);
    }

    if (bpg_mgr->m_ctx_init) {
        if (bpg_mgr->m_ctx_init(context, NULL, bpg_mgr->m_ctx_ctx) != 0) {
            CPE_ERROR(
                em, "%s.%s: create op: use-ctx-init: init fail!",
                bpg_rsp_manage_name(bpg_mgr), rsp_name);
            logic_context_free(context);
            return NULL;
        }
    }

    if (logic_context_bind(
            context,
            logic_executor_ref_executor(rsp->m_executor_ref)) != 0)
    {
        CPE_ERROR(
            em, "%s.%s: create op: bind executor to context fail!",
            bpg_rsp_manage_name(bpg_mgr), rsp_name);
        bpg_rsp_manage_free_context(bpg_mgr, context);
        return NULL;
    }

    next_op = bpg_rsp_queue_context(bpg_mgr, rsp, context, bpg_rsp_context_client_id(carry_info), em);
    switch(next_op) {
    case bpg_rsp_queue_next_op_success:
    case bpg_rsp_queue_next_op_exec:
        logic_context_set_commit(context, bpg_rsp_commit, rsp);
        break;
    case bpg_rsp_queue_next_op_error:
        bpg_rsp_manage_free_context(bpg_mgr, context);
        return NULL;
    default:
        CPE_ERROR(
            em, "%s.%s: create op: unknown next op %d!",
            bpg_rsp_manage_name(bpg_mgr), rsp_name, next_op);
        bpg_rsp_manage_free_context(bpg_mgr, context);
        return NULL;
    }

    return context;
}

static int bpg_rsp_copy_carry_metas(
    bpg_rsp_manage_t bpg_mgr, const char * rsp_name, logic_context_t from_context, logic_context_t to_context, error_monitor_t em)
{
    logic_data_t carry_info_list_data;
    BPG_CARRY_METAS * carry_info_list;
    uint32_t i;

    carry_info_list_data = logic_context_data_find(from_context, "bpg_carry_metas");
    if (carry_info_list_data == NULL) return 0;

    carry_info_list = logic_data_data(carry_info_list_data);

    for(i = 0; i < carry_info_list->count; ++i) {
        logic_data_t carry_data = logic_context_data_find(from_context, carry_info_list->data[i].name);

        if (carry_data == NULL) {
            CPE_ERROR(
                em, "%s.%s: copy_carry data %s: data not exist!",
                bpg_rsp_manage_name(bpg_mgr), rsp_name, carry_info_list->data[i].name);
            return -1;
        }

        if (logic_context_data_copy(to_context, carry_data) == NULL) {
            CPE_ERROR(
                em, "%s.%s: copy_carry data %s: create req_data fial!",
                bpg_rsp_manage_name(bpg_mgr), rsp_name, carry_info_list->data[i].name);
            return -1;
        }
    }

    return 0;
}
