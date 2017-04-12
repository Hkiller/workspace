#include <limits>
#include "cpepp/utils/CString.hpp"
#include "cpepp/cfg/Node.hpp"
#include "cpepp/nm/Manager.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Module.hpp"
#include "gdpp/app/Application.hpp"
#include "usf/logic/logic_executor.h"
#include "usf/logic/logic_executor_type.h"
#include "usf/logic/logic_manage.h"
#include "usf/logic/logic_context.h"
#include "usfpp/logic/LogicOp.hpp"
#include "usfpp/logic/LogicOpStack.hpp"
#include "usfpp/logic/LogicOpTypeGroup.hpp"

namespace Usf { namespace Logic {

LogicOp::LogicOp(execute_fun_t fun) : m_exec_fun(fun) {
}

LogicOp &
LogicOp::get(gd_app_context_t app, cpe_hash_string_t name) {
    LogicOp * r =
        dynamic_cast<LogicOp *>(
            &Gd::App::Application::_cast(app).nmManager().object(name));
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(app, ::std::runtime_error, "LogicOp %s cast fail!", cpe_hs_data(name));
    }
    return *r;
}

LogicOp &
LogicOp::get(gd_app_context_t app, const char * name) {
    LogicOp * r =
        dynamic_cast<LogicOp *>(
            &Gd::App::Application::_cast(app).nmManager().objectNc(name));
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(app, ::std::runtime_error, "LogicOp %s cast fail!", name);
    }
    return *r;
}

void LogicOp::regist_to(logic_executor_type_group_t group) {
    logic_executor_type_t type = logic_executor_type_create(group, name());
    if (type == NULL) {
        APP_CTX_THROW_EXCEPTION(
            logic_executor_type_group_app(group),
            ::std::runtime_error,
            "logic op type %s already exist in group %s!",
            name(), logic_executor_type_group_name(group));
    }

    logic_executor_type_bind(type, logic_op_adapter, this, NULL);
}

logic_op_exec_result_t LogicOp::logic_op_adapter(logic_context_t ctx, logic_stack_node_t stack_node, void * user_data, cfg_t cfg) {
    LogicOp * op = (LogicOp*)user_data;
    logic_executor_t executor = logic_stack_node_executor(stack_node);

    try {
        logic_op_exec_result_t rv = (op->*(op->m_exec_fun))(*(LogicOpContext*)ctx, *(LogicOpStackNode*)stack_node, Cpe::Cfg::Node::_cast(cfg));

        if (logic_context_flag_is_enable(ctx, logic_context_flag_debug)) {
            APP_CTX_INFO(
                logic_context_app(ctx), "execute logic op %s: complete, errno=%d, state=%d",
                logic_executor_name(executor), logic_context_errno(ctx), logic_context_state(ctx));
        }

        return rv;
    }
    APP_CTX_CATCH_EXCEPTION(logic_context_app(ctx), "%s: execute: ", logic_executor_name(executor));
    return logic_op_exec_result_null;
}

void LogicOp::init(LogicOp * product, Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & moduleCfg) {
    if (product == NULL) return;

    Cpe::Cfg::Node const & registerToCfg = moduleCfg["regist-to-group"];
    if (registerToCfg.isValid()) {
        if (registerToCfg.type() == CPE_CFG_TYPE_SEQUENCE) {
        }
        else if (registerToCfg.type() == CPE_CFG_TYPE_STRING) {
            product->regist_to(LogicOpTypeGroup::instance(app, registerToCfg.asString().c_str()));
        }
        else {
            APP_CTX_THROW_EXCEPTION(
                app,
                ::std::runtime_error,
                "logic op %s register error, 'regist-to-group' should be string or sequence, but is %d!",
                module.name(), registerToCfg.type());
        }
    }
    else {
        product->regist_to(LogicOpTypeGroup::instance(app));
    }
}

}}
