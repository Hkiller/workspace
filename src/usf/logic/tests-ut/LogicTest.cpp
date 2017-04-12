#include <stdexcept>
#include "usf/logic/logic_executor_type.h"
#include "LogicTest.hpp"

void LogicTest::SetUp() {
    Base::SetUp();
    testing::DefaultValue<logic_op_exec_result>::Set(logic_op_exec_result_null);
}

void LogicTest::TearDown() {
    testing::DefaultValue<logic_op_exec_result>::Clear();

    Base::TearDown();
}

static void ctx_free(void * ctx) {
    delete (LogicTest::LogicOpMock*)ctx;
}

static logic_op_exec_result_t execute_fun (logic_context_t ctx, logic_stack_node_t stack_node, void * user_data, cfg_t cfg) {
    LogicTest::LogicOpMock * op = (LogicTest::LogicOpMock *)user_data;
    return op->execute(stack_node);
}

LogicTest::LogicOpMock &
LogicTest::installOp(const char * name) {
    logic_executor_type_group_t group = t_logic_executor_type_group(NULL);

    logic_executor_type_t type = logic_executor_type_find(group, name);
    EXPECT_TRUE(type == NULL) << "logic op " << name << " already exist!";
    if (type) {
        return *(LogicTest::LogicOpMock*)logic_executor_type_ctx(type);
    }

    type = logic_executor_type_create(group, name);
    EXPECT_TRUE(type != NULL) << "logic op " << name << " create fail";
    if (type == NULL) {
        throw ::std::runtime_error("logic op not exist!");
    }

    EXPECT_EQ(0, logic_executor_type_bind(type, execute_fun, new LogicOpMock, ctx_free));

    return *(LogicTest::LogicOpMock*)logic_executor_type_ctx(type);
}

LogicTest::LogicOpMock &
LogicTest::op(const char * name) {
    logic_executor_type_group_t group = t_logic_executor_type_group(NULL);

    logic_executor_type_t type = logic_executor_type_find(group, name);
    EXPECT_TRUE(type) << "logic op " << name << " not exist!";
    if (type == NULL) {
        throw ::std::runtime_error("logic op not exist!");
    }

    return *(LogicTest::LogicOpMock*)logic_executor_type_ctx(type);
}

static void commit_to_mock(logic_context_t ctx, void * user_data) {
    ((LogicTest::CommitMock *)user_data)->commit(ctx);
}

void LogicTest::set_commit(logic_context_t context, CommitMock & mock) {
    logic_context_set_commit(context, commit_to_mock, &mock);
}

