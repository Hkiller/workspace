#ifndef USFPP_LOGIC_LOGICOP_H
#define USFPP_LOGIC_LOGICOP_H
#include "cpepp/nm/Object.hpp"
#include "System.hpp"

namespace Usf { namespace Logic {

class LogicOp : public Cpe::Nm::Object {
public:
    struct R {
        R(bool v) : m_result(v ? logic_op_exec_result_true : logic_op_exec_result_false) {}
        R(logic_op_exec_result_t v) : m_result(v) {}

        operator logic_op_exec_result_t () const { return m_result; };
        bool operator==(logic_op_exec_result_t r) const { return m_result == r; }
        bool operator!=(logic_op_exec_result_t r) const { return m_result != r; }
        bool operator==(bool r) const { return m_result == (r ? logic_op_exec_result_true : logic_op_exec_result_false); }
        bool operator!=(bool r) const { return m_result != (r ? logic_op_exec_result_true : logic_op_exec_result_false); }
        bool operator==(R const & o) const { return m_result == o.m_result; }
        bool operator!=(R const & o) const { return m_result != o.m_result; }
        operator bool() const { return m_result == logic_op_exec_result_true; }
    private:
        R();

        logic_op_exec_result_t m_result;
    };

    typedef R (LogicOp::*execute_fun_t)(LogicOpContext & context, LogicOpStackNode & stackNode, Cpe::Cfg::Node const & cfg) const;

    LogicOp(execute_fun_t fun);

    void regist_to(logic_executor_type_group_t group);

    static LogicOp & get(gd_app_context_t app, cpe_hash_string_t name);
    static LogicOp & get(gd_app_context_t app, const char * name);

    static void init(
        LogicOp * product,
        Gd::App::Application & app,
        Gd::App::Module & module,
        Cpe::Cfg::Node & moduleCfg);

    static R _null(void) { return logic_op_exec_result_null; }

private:
    execute_fun_t m_exec_fun;

    static logic_op_exec_result_t logic_op_adapter(logic_context_t ctx, logic_stack_node_t stack_node, void * user_data, cfg_t cfg);
};

template<typename OutT, typename ContextT>
class LogicOpDef : public LogicOp {
public:
    LogicOpDef() : LogicOp((execute_fun_t)&LogicOpDef::execute) {}

    virtual R execute(
        ContextT & context,
        LogicOpStackNode & stackNode,
        Cpe::Cfg::Node const & cfg) const = 0;
};

}}

#endif
