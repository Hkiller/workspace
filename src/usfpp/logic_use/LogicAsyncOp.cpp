#include <limits>
#include "usf/logic/logic_executor.h"
#include "usf/logic/logic_context.h"
#include "gdpp/app/Log.hpp"
#include "cpepp/cfg/Node.hpp"
#include "usf/logic/logic_stack.h"
#include "usf/logic_use/logic_op_async.h"
#include "usfpp/logic/LogicOpContext.hpp"
#include "usfpp/logic/LogicOpStack.hpp"
#include "usfpp/logic_use/LogicAsyncOp.hpp"

namespace Usf { namespace Logic {

LogicAsyncOp::LogicAsyncOp(execute_fun_t send_fun, recv_fun_t recv_fun)
    : Logic::LogicOp((execute_fun_t)&LogicAsyncOp::execute)
    , m_send_fun(send_fun)
    , m_recv_fun(recv_fun)
{
}

logic_op_exec_result_t LogicAsyncOp::send_op_adapter(logic_context_t ctx, logic_stack_node_t stack_node, void * user_data, cfg_t cfg) {
    LogicAsyncOp * op = (LogicAsyncOp*)user_data;
    logic_executor_t executor = logic_stack_node_executor(stack_node);
    const char * executor_name = logic_executor_name(executor);
    try {
        logic_op_exec_result_t rv = (op->*(op->m_send_fun))(*(Logic::LogicOpContext*)ctx, *(Logic::LogicOpStackNode*)stack_node, Cpe::Cfg::Node::_cast(cfg));

        if (logic_context_flag_is_enable(ctx, logic_context_flag_debug)) {
            APP_CTX_INFO(
                logic_context_app(ctx), "%s: async send: complete, errno=%d, state=%d",
                executor_name, logic_context_errno(ctx), logic_context_state(ctx));
        }

        return rv;
    }
    APP_CTX_CATCH_EXCEPTION(logic_context_app(ctx), "%s: async send: ", executor_name);
    return logic_op_exec_result_null;
}

logic_op_exec_result_t
LogicAsyncOp::recv_op_adapter(
    logic_context_t ctx,
    logic_stack_node_t stack_node,
    logic_require_t require,
    void * user_data, cfg_t cfg)
{
    LogicAsyncOp * op = (LogicAsyncOp*)user_data;
    logic_executor_t executor = logic_stack_node_executor(stack_node);
    const char * executor_name = logic_executor_name(executor);
    try {
        logic_op_exec_result_t rv = (op->*(op->m_recv_fun))(
            *(Logic::LogicOpContext*)ctx,
            *(Logic::LogicOpStackNode*)stack_node,
            *(Logic::LogicOpRequire*)require,
            Cpe::Cfg::Node::_cast(cfg));

        if (logic_context_flag_is_enable(ctx, logic_context_flag_debug)) {
            APP_CTX_INFO(
                logic_context_app(ctx), "%s: async recv: complete, errno=%d, state=%d",
                executor_name, logic_context_errno(ctx), logic_context_state(ctx));
        }

        return rv;
    }
    APP_CTX_CATCH_EXCEPTION(logic_context_app(ctx), "%s: async recv: ", executor_name);
    return logic_op_exec_result_null;
}

LogicAsyncOp::R LogicAsyncOp::execute(Logic::LogicOpContext & context, Logic::LogicOpStackNode & stackNode, Cpe::Cfg::Node const & cfg) const {
    return logic_op_asnyc_exec(context, stackNode, send_op_adapter, recv_op_adapter, (void*)this, cfg);
}

}}

