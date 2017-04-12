#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/dp/dp_binding.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dr/dr_data.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_context.h"
#include "usf/logic/logic_queue.h"
#include "usf/logic/logic_executor_ref.h"
#include "usf/logic/logic_data.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/set/logic/set_logic_rsp_manage.h"
#include "svr/set/logic/set_logic_rsp.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "set_logic_rsp_ops.h"
#include "protocol/set/logic/set_logic_rsp_carry_info.h"

extern char g_metalib_set_logic_data_meta[];

enum set_logic_rsp_queue_next_op {
    set_logic_rsp_queue_next_op_success
    , set_logic_rsp_queue_next_op_exec
    , set_logic_rsp_queue_next_op_error
};

static void set_logic_rsp_commit_error(set_logic_rsp_t rsp, logic_context_t op_context, int err);
static enum set_logic_rsp_queue_next_op set_logic_rsp_queue_context(set_logic_rsp_manage_t bpg_mgr, set_logic_rsp_t rsp, logic_context_t op_context, uint64_t client_id, error_monitor_t em);
static int set_logic_rsp_copy_carry_metas(set_logic_rsp_manage_t bpg_mgr, const char * rsp_name, logic_context_t from_context, logic_context_t to_context, error_monitor_t em);

int set_logic_rsp_execute(dp_req_t dp_req, void * ctx, error_monitor_t em) {
    set_logic_rsp_t set_logic_rsp;
    set_logic_rsp_manage_t bpg_mgr;
    logic_context_t op_context;
    enum set_logic_rsp_queue_next_op next_op;

    set_logic_rsp = (set_logic_rsp_t)ctx;
    assert(set_logic_rsp);

    bpg_mgr = set_logic_rsp->m_mgr;
    assert(bpg_mgr);

    op_context = set_logic_rsp_manage_create_context(bpg_mgr, dp_req, em);
    if (op_context == NULL) {
        CPE_ERROR(
            em, "%s.%s: set_logic_rsp_execute: create op_context for pkg fail!",
            set_logic_rsp_manage_name(bpg_mgr), set_logic_rsp_name(set_logic_rsp));
        return 0;
    }

    if (set_logic_rsp_flag_is_enable(set_logic_rsp, set_logic_rsp_flag_debug)) {
        logic_context_flag_enable(op_context, logic_context_flag_debug);
    }

    if (logic_context_bind(
            op_context,
            logic_executor_ref_executor(set_logic_rsp->m_executor_ref)) != 0)
    {
        CPE_ERROR(
            em, "%s.%s: set_logic_rsp_execute: bind executor to context fail!",
            set_logic_rsp_manage_name(bpg_mgr), set_logic_rsp_name(set_logic_rsp));
        set_logic_rsp_commit_error(set_logic_rsp, op_context, -1);
        return 0;
    }

    next_op = set_logic_rsp_queue_next_op_exec;

    if (bpg_mgr->m_queue_attr) {
        uint64_t client_id;
        if (logic_context_try_read_uint64(&client_id, op_context, bpg_mgr->m_queue_attr, em) != 0) {
            CPE_ERROR(
                em, "%s.%s: create op: read client id %s fail!",
                set_logic_rsp_manage_name(bpg_mgr), set_logic_rsp_name(set_logic_rsp), bpg_mgr->m_queue_attr);
            set_logic_rsp_commit_error(set_logic_rsp, op_context, -1);
            return 0;
        }
        
        next_op = set_logic_rsp_queue_context(bpg_mgr, set_logic_rsp, op_context, client_id, em);
    }

    switch(next_op) {
    case set_logic_rsp_queue_next_op_success:
        logic_context_set_commit(op_context, set_logic_rsp_commit, set_logic_rsp);
        break;
    case set_logic_rsp_queue_next_op_exec:
        logic_context_set_commit(op_context, set_logic_rsp_commit, set_logic_rsp);
        logic_context_execute(op_context);
        break;
    case set_logic_rsp_queue_next_op_error:
        set_logic_rsp_commit_error(set_logic_rsp, op_context, -1);
        break;
    default:
        CPE_ERROR(
            em, "%s.%s: set_logic_rsp_execute: unknown next op %d!",
            set_logic_rsp_manage_name(bpg_mgr), set_logic_rsp_name(set_logic_rsp), next_op);
        set_logic_rsp_commit_error(set_logic_rsp, op_context, -1);
        return 0;
    }

    return 0;
}

static logic_queue_t
set_logic_rsp_queue_get_or_create(
    set_logic_rsp_manage_t mgr, set_logic_rsp_t rsp, 
    cpe_hash_string_t queue_name, struct set_logic_rsp_queue_info * queue_info, error_monitor_t em)
{
    logic_queue_t queue = logic_queue_find(mgr->m_logic_mgr, queue_name);
    if (queue) return queue;

    queue = logic_queue_create(mgr->m_logic_mgr, cpe_hs_data(queue_name));
    if (queue == NULL) {
        CPE_ERROR(
            em, "%s.%s: set_logic_rsp_execute: create queue(%s) fail!",
            set_logic_rsp_manage_name(mgr), set_logic_rsp_name(rsp), cpe_hs_data(queue_name));
        return NULL;
    }

    logic_queue_set_max_count(queue, queue_info->m_max_count);

    if (mgr->m_debug) {
        CPE_INFO(
            em, "%s.%s: set_logic_rsp_execute: create queue(%s) success, max-size=%d, size=%d!",
            set_logic_rsp_manage_name(mgr), set_logic_rsp_name(rsp),
            logic_queue_name(queue), logic_queue_max_count(queue), logic_queue_count(queue));
    }

    return queue;
}

static enum set_logic_rsp_queue_next_op set_logic_rsp_queue_context(
    set_logic_rsp_manage_t mgr, set_logic_rsp_t rsp, logic_context_t op_context, uint64_t client_id, error_monitor_t em)
{
    struct set_logic_rsp_queue_info * queue_info = rsp->m_queue_info;
    logic_queue_t queue;

    if (queue_info == NULL) return set_logic_rsp_queue_next_op_exec;

    switch(queue_info->m_scope) {
    case set_logic_rsp_queue_scope_global:
        queue = set_logic_rsp_queue_get_or_create(mgr, rsp, queue_info->m_name, queue_info, em);
        break;
    case set_logic_rsp_queue_scope_client: {
        cpe_hs_printf(
            (cpe_hash_string_t)queue_info->m_name_buf,
            sizeof(queue_info->m_name_buf),
            "%s."FMT_SIZE_T, set_logic_rsp_queue_name(queue_info), client_id);
        queue = set_logic_rsp_queue_get_or_create(mgr, rsp, (cpe_hash_string_t)queue_info->m_name_buf, queue_info, em);
        break;
    }
    default:
        CPE_ERROR(
            em, "%s.%s: set_logic_rsp_execute: set_logic_rsp_queue_context: unknown scope type!",
            set_logic_rsp_manage_name(mgr), set_logic_rsp_name(rsp));
        return set_logic_rsp_queue_next_op_error;
    }

    if (queue) {
        logic_queue_enqueue_tail(queue, op_context);
        if (mgr->m_debug) {
            CPE_INFO(
                em, "%s.%s: set_logic_rsp_execute: add to queue(%s) tail, max-size=%d, size=%d!",
                set_logic_rsp_manage_name(mgr), set_logic_rsp_name(rsp),
                logic_queue_name(queue), logic_queue_max_count(queue), logic_queue_count(queue));
        }

        if (logic_queue_head(queue) == op_context) {
            return set_logic_rsp_queue_next_op_exec;
        }
        else {
            return set_logic_rsp_queue_next_op_success;
        }
    }
    else {
        CPE_ERROR(
            em, "%s.%s: set_logic_rsp_execute: add to queue fail!",
            set_logic_rsp_manage_name(mgr), set_logic_rsp_name(rsp));
        return set_logic_rsp_queue_next_op_error;
    }
}

static int set_logic_rsp_copy_req_main_to_ctx(
    set_logic_rsp_manage_t mgr, logic_context_t op_context,
    dp_req_t req, uint32_t cmd, error_monitor_t em)
{
    LPDRMETA data_meta;
    logic_data_t data;
    size_t size;
    size_t data_start;
    size_t data_size;
    size_t meta_size;

    data_meta = set_svr_svr_info_find_data_meta_by_cmd(mgr->m_svr_type, cmd);
    if (data_meta == NULL) return 0;

    size = dp_req_size(req);

    data_start = dr_entry_data_start_pos(mgr->m_pkg_data_entry, 0);
    if (data_start > size) {
        CPE_ERROR(
            em, "%s: copy req main to ctx: not enouth data, data-start-pos=%d, data-size=%d!",
            set_logic_rsp_manage_name(mgr), (int)data_start, (int)size);
        return -1;
    }

    data_size = size - data_start;
    meta_size = dr_meta_size(data_meta);

    data = logic_context_data_get_or_create(op_context, data_meta, data_size < meta_size ? meta_size : data_size);
    if (data == NULL) {
        CPE_ERROR(
            em, "%s: set_logic_rsp_execute: copy_pkg_to_ctx: %s create data fail, capacity=%d!",
            set_logic_rsp_manage_name(mgr), dr_meta_name(data_meta), (int)size);
        return -1;
    }

    memcpy(logic_data_data(data), ((const char *)dp_req_data(req)) + data_start, data_size);

    return 0;
}

static int set_logic_rsp_copy_req_carry_to_ctx(
    set_logic_rsp_manage_t mgr, logic_context_t op_context,
    dp_req_t bpg_req, uint16_t from_svr_type_id, error_monitor_t em)
{
    LPDRMETA carry_meta;
    logic_data_t data;
    dp_req_t pkg_carry;
    const void * carry_data;
    size_t carry_size;
    LPDRMETA set_carry_data_meta;
    set_svr_svr_info_t from_svr_type;

    pkg_carry = set_pkg_carry_find(bpg_req);
    if (pkg_carry == NULL) {
        CPE_ERROR(
            em, "%s: copy_set_logic_carry_data: no carry pkg!",
            set_logic_rsp_manage_name(mgr));
        return -1;
    }

    from_svr_type = set_svr_svr_info_find(mgr->m_stub, from_svr_type_id);
    set_carry_data_meta = from_svr_type ? set_svr_svr_info_carry_meta(from_svr_type) : NULL;

    if (set_carry_data_meta) {
        carry_meta = set_carry_data_meta;
        carry_size = (size_t)set_pkg_carry_size(pkg_carry);
        carry_data = set_pkg_carry_data(pkg_carry);
    }
    else {
        carry_meta = dp_req_meta(pkg_carry);
        carry_size = dp_req_size(pkg_carry);
        carry_data = dp_req_data(pkg_carry);
    }

    data = logic_context_data_get_or_create(op_context, carry_meta, carry_size);
    if (data == NULL) {
        CPE_ERROR(
            em, "%s: copy_req_carry_data: %s create data fail, size=%d!",
            set_logic_rsp_manage_name(mgr),
            dr_meta_name(carry_meta), (int)carry_size);
        return -1;
    }

    memcpy(logic_data_data(data), carry_data, carry_size);

    return 0;
}

static SET_LOGIC_CARRY_INFO *
set_logic_rsp_create_carry_info(set_logic_rsp_manage_t mgr, logic_context_t op_context, dp_req_t pkg_head, dp_req_t pkg_body, error_monitor_t em) {
    LPDRMETA set_logic_carry_data_meta;
    logic_data_t data;
    SET_LOGIC_CARRY_INFO * buf;

    set_logic_carry_data_meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_set_logic_data_meta, "set_logic_carry_info");
    if (set_logic_carry_data_meta == NULL) {
        CPE_ERROR(
            em, "%s: copy_set_logic_carry_data: set_logic_carry_info meta not exist!",
            set_logic_rsp_manage_name(mgr));
        return NULL;
    }

    data = logic_context_data_get_or_create(op_context, set_logic_carry_data_meta, dr_meta_size(set_logic_carry_data_meta));
    if (data == NULL) {
        CPE_ERROR(
            em, "%s: copy_set_logic_carry_data: %s create data fail, size=%d!",
            set_logic_rsp_manage_name(mgr),
            dr_meta_name(set_logic_carry_data_meta), (int)dr_meta_size(set_logic_carry_data_meta));
        return NULL;
    }

    buf = (SET_LOGIC_CARRY_INFO *)logic_data_data(data);

    if (pkg_head) {
        buf->sn = set_pkg_sn(pkg_head);
        buf->cmd = 0;
        buf->response = 0;
        buf->from_svr_type = set_pkg_from_svr_type(pkg_head);
        buf->from_svr_id = set_pkg_from_svr_id(pkg_head);

        if (dr_entry_try_read_uint32(
                &buf->cmd, ((const char*)dp_req_data(pkg_body)) + dr_entry_data_start_pos(mgr->m_pkg_cmd_entry, 0),
                mgr->m_pkg_cmd_entry, em) != 0)
        {
            CPE_ERROR(
                em, "%s: copy_set_logic_carry_data: read cmd from %s fail!",
                set_logic_rsp_manage_name(mgr), dr_entry_name(mgr->m_pkg_cmd_entry));
            logic_data_free(data);
            return NULL;
        }

        if (buf->cmd != 0) buf->response = buf->cmd + 1;
    }
    else {
        bzero(buf, sizeof(*buf));
    }

    return buf;
}

static void set_logic_rsp_commit_error(set_logic_rsp_t rsp, logic_context_t op_context, int err) {
    if (rsp->m_mgr->m_debug) {
        CPE_INFO(
            gd_app_em(rsp->m_mgr->m_app), "%s.%s: commit error, error=%d!",
            set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp), err);
    }

    logic_context_errno_set(op_context, err);
    set_logic_rsp_commit(op_context, rsp);
}

logic_context_t set_logic_rsp_manage_create_context(set_logic_rsp_manage_t bpg_mgr, dp_req_t req, error_monitor_t em) {
    logic_context_t op_context;
    dp_req_t pkg_head;
    SET_LOGIC_CARRY_INFO * carry_info = NULL;
    uint32_t sn = INVALID_LOGIC_CONTEXT_ID;

    pkg_head = req ? set_pkg_head_find(req) : NULL;

    if (pkg_head && set_logic_rsp_manage_flag_is_enable(bpg_mgr, set_logic_rsp_manage_flag_sn_use_client)) {
        sn = set_pkg_sn(pkg_head);
    }

    op_context = logic_context_create(bpg_mgr->m_logic_mgr, sn, bpg_mgr->m_ctx_capacity);
    if (op_context == NULL) {
        CPE_ERROR(
            em, "%s: create context: fail, capacity is %d!",
            set_logic_rsp_manage_name(bpg_mgr), (int)bpg_mgr->m_ctx_capacity);
        return NULL;
    }

    carry_info = set_logic_rsp_create_carry_info(bpg_mgr, op_context, pkg_head, req, em);
    if (carry_info == NULL) {
        logic_context_free(op_context);
        return NULL;
    }

    if (req) {
        /*aft user init, we should commit any error*/
        if (set_logic_rsp_copy_req_main_to_ctx(bpg_mgr, op_context, req, carry_info->cmd, em) != 0) {
            CPE_ERROR(em, "%s: create context: copy main to context fail!", set_logic_rsp_manage_name(bpg_mgr));
            logic_context_free(op_context);
            return NULL;
        }

        if (set_logic_rsp_copy_req_carry_to_ctx(bpg_mgr, op_context, req, carry_info->from_svr_type, em) != 0) {
            CPE_ERROR(em, "%s: create context: copy carry to context fail!", set_logic_rsp_manage_name(bpg_mgr));
            logic_context_free(op_context);
            return NULL;
        }
    }

    if (bpg_mgr->m_debug >= 2) {
        logic_context_flag_enable(op_context, logic_context_flag_debug);
    }

    if (bpg_mgr->m_ctx_init) {
        if (bpg_mgr->m_ctx_init(op_context, req, bpg_mgr->m_ctx_ctx) != 0) {
            CPE_ERROR(
                em, "%s: create context: use-ctx-init: init fail!",
                set_logic_rsp_manage_name(bpg_mgr));
            logic_context_free(op_context);
            return NULL;
        }
    }
    
    return op_context;
}

logic_context_t
set_logic_rsp_manage_create_follow_op_by_name(set_logic_rsp_manage_t bpg_mgr, logic_context_t context, const char * rsp_name, error_monitor_t em) {
    set_logic_rsp_t rsp;
    dp_rsp_t dp_rsp;
    logic_context_t follow_context;
    logic_data_t input_carry_data;
    logic_data_t carry_data;
    set_logic_rsp_carry_info_t carry_info;

    assert(rsp_name);

    rsp = set_logic_rsp_find(bpg_mgr, rsp_name);
    if (rsp == NULL) {
        CPE_ERROR(
            em, "%s.%s: create follow op: fail, rsp not exist!",
            set_logic_rsp_manage_name(bpg_mgr), rsp_name);
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
            set_logic_rsp_manage_name(bpg_mgr), rsp_name, (int)bpg_mgr->m_ctx_capacity);
        return NULL;
    }

    if (context) {
        if (set_logic_rsp_copy_carry_metas(bpg_mgr, rsp_name, context, follow_context, em) != 0) {
            CPE_ERROR(
                em, "%s.%s: create follow op: fail, create carry metas fail!",
                set_logic_rsp_manage_name(bpg_mgr), rsp_name);
            logic_context_free(follow_context);
            return NULL;
        }
    }

    if ((input_carry_data = logic_context_data_find(context, "set_logic_carry_info"))) {
        carry_data = logic_context_data_copy(follow_context, input_carry_data);
    }
    else {
        CPE_ERROR(
            em, "%s.%s: create follow op: set_logic_carry_info not exist in input context!",
            set_logic_rsp_manage_name(bpg_mgr), rsp_name);
        logic_context_free(follow_context);
        return NULL;
    }

    carry_info = (set_logic_rsp_carry_info_t)logic_data_data(carry_data);
    assert(carry_info);

    if ((dp_rsp = set_logic_rsp_dp(rsp))) {
        struct dp_binding_it binding_it;
        dp_binding_t only_binding;

        dp_rsp_bindings(&binding_it, dp_rsp);
        only_binding = dp_binding_next(&binding_it);
        if (only_binding && dp_binding_next(&binding_it) == NULL) {
            uint32_t cmd;
            if (dp_binding_numeric(&cmd, only_binding) == 0) {
                assert(carry_data != NULL);
                set_logic_rsp_context_set_cmd(carry_info, cmd);
            }
        }
        else {
            set_logic_rsp_context_set_response(carry_info, 0);
        }
    }
    else {
        set_logic_rsp_context_set_response(carry_info, 0);
    }

    if (bpg_mgr->m_debug >= 2 || set_logic_rsp_flag_is_enable(rsp, set_logic_rsp_flag_debug)) {
        logic_context_flag_enable(follow_context, logic_context_flag_debug);
    }

    if (bpg_mgr->m_ctx_init) {
        if (bpg_mgr->m_ctx_init(follow_context, NULL, bpg_mgr->m_ctx_ctx) != 0) {
            CPE_ERROR(
                em, "%s.%s: create follow op: use-ctx-init: init fail!",
                set_logic_rsp_manage_name(bpg_mgr), rsp_name);
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
            set_logic_rsp_manage_name(bpg_mgr), rsp_name);
        set_logic_rsp_manage_free_context(bpg_mgr, follow_context);
        return NULL;
    }

    if (logic_context_queue(context)) {
        if (logic_queue_enqueue_after(context, follow_context) != 0) {
            CPE_ERROR(
                em, "%s.%s: create follow op: enqueue after input fail!",
                set_logic_rsp_manage_name(bpg_mgr), rsp_name);
            set_logic_rsp_manage_free_context(bpg_mgr, follow_context);
            return NULL;
        }
    }

    logic_context_set_commit(follow_context, set_logic_rsp_commit, rsp);

    return follow_context;
}

logic_context_t
set_logic_rsp_manage_create_op_by_name(
    set_logic_rsp_manage_t bpg_mgr,
    logic_context_t from_context, 
    const char * rsp_name,
    error_monitor_t em)
{
    set_logic_rsp_t rsp;
    dp_rsp_t dp_rsp;
    logic_context_t context;
    logic_data_t input_carry_data;
    logic_data_t carry_data;
    set_logic_rsp_carry_info_t carry_info;
    enum set_logic_rsp_queue_next_op next_op;

    assert(rsp_name);

    rsp = set_logic_rsp_find(bpg_mgr, rsp_name);
    if (rsp == NULL) {
        CPE_ERROR(
            em, "%s.%s: create op: fail, rsp not exist!",
            set_logic_rsp_manage_name(bpg_mgr), rsp_name);
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
            set_logic_rsp_manage_name(bpg_mgr), rsp_name, (int)bpg_mgr->m_ctx_capacity);
        return NULL;
    }

    if ((input_carry_data =
         from_context
         ? logic_context_data_find(from_context, "set_logic_carry_info")
         : NULL))
    {
        carry_data = logic_context_data_copy(context, input_carry_data);
    }
    else {
        if (set_logic_rsp_create_carry_info(bpg_mgr, context, NULL, NULL, em) == NULL) {
            CPE_ERROR(
                em, "%s.%s: create op: fail, create carry info fail!",
                set_logic_rsp_manage_name(bpg_mgr), rsp_name);
            logic_context_free(context);
            return NULL;
        }
    }

    if (from_context) {
        if (set_logic_rsp_copy_carry_metas(bpg_mgr, rsp_name, from_context, context, em) != 0) {
            CPE_ERROR(
                em, "%s.%s: create op: fail, create carry metas fail!",
                set_logic_rsp_manage_name(bpg_mgr), rsp_name);
            logic_context_free(context);
            return NULL;
        }
    }

    carry_data = logic_context_data_find(context, "set_logic_carry_info");
    assert(carry_data);

    carry_info = (set_logic_rsp_carry_info_t)logic_data_data(carry_data);
    assert(carry_info);

    if ((dp_rsp = set_logic_rsp_dp(rsp))) {
        struct dp_binding_it binding_it;
        dp_binding_t only_binding;

        dp_rsp_bindings(&binding_it, dp_rsp);
        only_binding = dp_binding_next(&binding_it);
        if (only_binding && dp_binding_next(&binding_it) == NULL) {
            uint32_t cmd;
            if (dp_binding_numeric(&cmd, only_binding) == 0) {
                set_logic_rsp_context_set_cmd(carry_info, cmd);
            }
        }
    }
    else {
        set_logic_rsp_context_set_response(carry_info, 0);
    }

    if (bpg_mgr->m_debug >= 2 || set_logic_rsp_flag_is_enable(rsp, set_logic_rsp_flag_debug)) {
        logic_context_flag_enable(context, logic_context_flag_debug);
    }

    if (bpg_mgr->m_ctx_init) {
        if (bpg_mgr->m_ctx_init(context, NULL, bpg_mgr->m_ctx_ctx) != 0) {
            CPE_ERROR(
                em, "%s.%s: create op: use-ctx-init: init fail!",
                set_logic_rsp_manage_name(bpg_mgr), rsp_name);
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
            set_logic_rsp_manage_name(bpg_mgr), rsp_name);
        set_logic_rsp_manage_free_context(bpg_mgr, context);
        return NULL;
    }

    next_op = set_logic_rsp_queue_next_op_exec;

    if (bpg_mgr->m_queue_attr) {
        uint64_t client_id;
        if (logic_context_try_read_uint64(&client_id, context, bpg_mgr->m_queue_attr, em) == 0) {
            next_op = set_logic_rsp_queue_context(bpg_mgr, rsp, context, client_id, em);
        }
    }

    switch(next_op) {
    case set_logic_rsp_queue_next_op_success:
    case set_logic_rsp_queue_next_op_exec:
        logic_context_set_commit(context, set_logic_rsp_commit, rsp);
        break;
    case set_logic_rsp_queue_next_op_error:
        set_logic_rsp_manage_free_context(bpg_mgr, context);
        return NULL;
    default:
        CPE_ERROR(
            em, "%s.%s: create op: unknown next op %d!",
            set_logic_rsp_manage_name(bpg_mgr), rsp_name, next_op);
        set_logic_rsp_manage_free_context(bpg_mgr, context);
        return NULL;
    }

    return context;
}

static int set_logic_rsp_copy_carry_metas(
    set_logic_rsp_manage_t bpg_mgr, const char * rsp_name, logic_context_t from_context, logic_context_t to_context, error_monitor_t em)
{
    logic_data_t bpg_private_data;
    SET_LOGIC_CARRY_INFO * bpg_private;
    LPDRMETA set_carry_data_meta;
    set_svr_svr_info_t from_svr_type;
    const char * carry_meta_name;
    logic_data_t carry_data;

    bpg_private_data = logic_context_data_find(from_context, "set_logic_carry_info");
    if (bpg_private_data == NULL) {
        return 0;
    }

    if (logic_context_data_copy(to_context, bpg_private_data) == NULL) {
        CPE_ERROR(
            em, "%s.%s: copy_carry data set_logic_carry_info fial!",
            set_logic_rsp_manage_name(bpg_mgr), rsp_name);
        return -1;
    }

    bpg_private = (SET_LOGIC_CARRY_INFO *)logic_data_data(bpg_private_data);

    from_svr_type = set_svr_svr_info_find(bpg_mgr->m_stub, bpg_private->from_svr_type);
    set_carry_data_meta = from_svr_type ? set_svr_svr_info_carry_meta(from_svr_type) : NULL;
    carry_meta_name = set_carry_data_meta ? dr_meta_name(set_carry_data_meta) : req_type_set_pkg_carry;

    carry_data = logic_context_data_find(from_context, carry_meta_name);
    if (carry_data) {
        if (logic_context_data_copy(to_context, carry_data) == NULL) {
            CPE_ERROR(
                em, "%s.%s: copy_carry data %s fail!",
                set_logic_rsp_manage_name(bpg_mgr), rsp_name, carry_meta_name);
            return -1;
        }
    }

    return 0;
}
