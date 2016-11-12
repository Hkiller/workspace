#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_log.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_context.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_stack.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_executor.h"
#include "usf/logic/logic_executor_type.h"
#include "usf/logic_use/logic_op_async.h"
#include "protocol/logic_use/logic_op_async_info.h"

struct logic_op_async_ctx {
    mem_allocrator_t m_alloc;
    logic_op_fun_t m_send_fun;
    logic_op_recv_fun_t m_recv_fun;
    void * m_user_data;
    logic_op_ctx_fini_fun_t m_fini_fun;
};

extern char g_metalib_logic_use[];

logic_op_exec_result_t
logic_op_asnyc_exec(
    logic_context_t context,
    logic_stack_node_t stack_node,
    logic_op_fun_t send_fun,
    logic_op_recv_fun_t recv_fun,
    void * user_data,
    cfg_t args)
{
    struct logic_require_it require_it;
    logic_require_t require;
    logic_executor_t executor;
    logic_op_exec_result_t tmp_rv;
    LPDRMETA meta;
    logic_data_t asnyc_info_data;
    LOGIC_OP_ASNYC_INFO * asnyc_info;

    assert(stack_node);

    executor = logic_stack_node_executor(stack_node);
    assert(executor);

    asnyc_info_data = logic_stack_data_find(stack_node, "logic_op_asnyc_info");
    if (asnyc_info_data == NULL) {
        meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_logic_use, "logic_op_asnyc_info");
        if (meta == NULL) {
            APP_CTX_ERROR(
                logic_context_app(context),
                "logic_op_asnyc_exec: %s: meta not exist!",
                logic_executor_name(executor));
            return logic_op_exec_result_null;
        }

        asnyc_info_data = logic_stack_data_get_or_create(stack_node, meta, sizeof(LOGIC_OP_ASNYC_INFO));
        if (asnyc_info_data == NULL) {
            APP_CTX_ERROR(
                logic_context_app(context),
                "logic_op_asnyc_exec: %s: create asnyc_info_data fail!",
                logic_executor_name(executor));
            return logic_op_exec_result_null;
        }
        assert(logic_stack_data_find(stack_node, "logic_op_asnyc_info"));

        asnyc_info = (LOGIC_OP_ASNYC_INFO *)logic_data_data(asnyc_info_data);

        tmp_rv = send_fun(context, stack_node, user_data, args);
        if (tmp_rv == logic_op_exec_result_null) return logic_op_exec_result_null;

        asnyc_info->res_rv = tmp_rv;
    }
    else {
        asnyc_info = (LOGIC_OP_ASNYC_INFO *)logic_data_data(asnyc_info_data);
    }

    assert(asnyc_info);

    logic_stack_node_requires(stack_node, &require_it);
    for(require = logic_require_next(&require_it);
        require;
        require = logic_require_next(&require_it))
    {
        logic_require_state_t require_state;

        assert(logic_require_stack(require) == stack_node);

        require_state = logic_require_state(require);
        if (require_state == logic_require_state_waiting
            || require_state == logic_require_state_canceling)
        {
            continue;
        }

        logic_require_disconnect_to_stack(require);

        tmp_rv = recv_fun(context, stack_node, require, user_data, args);

        if (tmp_rv == logic_op_exec_result_null) return logic_op_exec_result_null;

        if (tmp_rv == logic_op_exec_result_false) asnyc_info->res_rv = logic_op_exec_result_false;

        logic_stack_node_requires(stack_node, &require_it);
    }

    /*最后检查还有没有require*/
    logic_stack_node_requires(stack_node, &require_it);

    return logic_require_next(&require_it) ? logic_op_exec_result_true : asnyc_info->res_rv;
}

static logic_op_exec_result_t
logic_op_asnyc(logic_context_t context, logic_stack_node_t stack_node, void * user_data, cfg_t args) {
    struct logic_op_async_ctx * ctx = (struct logic_op_async_ctx *)user_data;
    return logic_op_asnyc_exec(context, stack_node, ctx->m_send_fun, ctx->m_recv_fun, ctx->m_user_data, args);
};

static void logic_op_async_ctx_free(void * ctx) {
    struct logic_op_async_ctx * pkg_ctx = (struct logic_op_async_ctx *)ctx;
    if (pkg_ctx->m_fini_fun) pkg_ctx->m_fini_fun(pkg_ctx->m_user_data);
    mem_free(pkg_ctx->m_alloc, pkg_ctx);
}

logic_executor_type_t
logic_op_async_type_create(
    gd_app_context_t app,
    const char * group_name,
    const char * name,
    logic_op_fun_t send_fun,
    logic_op_recv_fun_t recv_fun,
    void * user_data,
    logic_op_ctx_fini_fun_t fini_fun,
    error_monitor_t em)
{
    logic_executor_type_t type;
    struct logic_op_async_ctx * ctx;


    ctx = mem_alloc(gd_app_alloc(app), sizeof(struct logic_op_async_ctx));
    if (ctx == NULL) {
        CPE_ERROR(em, "bpg_use_op_send_pkg_create: malloc ctx fail!");
        return NULL;
    }

    ctx->m_alloc = gd_app_alloc(app);
    ctx->m_send_fun = send_fun;
    ctx->m_recv_fun = recv_fun;
    ctx->m_user_data = user_data;
    ctx->m_fini_fun = fini_fun;

    type = logic_executor_type_create_global(
        app,
        group_name,
        name,
        logic_op_asnyc,
        ctx,
        logic_op_async_ctx_free,
        gd_app_em(app));

    if (type == NULL) {
        CPE_ERROR(em, "bpg_use_op_send_pkg_create: create type fail!");
        logic_op_async_ctx_free(ctx);
        return NULL;
    }

    return type;
}

