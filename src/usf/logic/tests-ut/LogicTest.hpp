#ifndef USF_LOGIC_TEST_LOGICTEST_H
#define USF_LOGIC_TEST_LOGICTEST_H
#include <string>
#include "gmock/gmock.h"
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "gd/app/tests-env/with_app.hpp"
#include "usf/logic/tests-env/with_logic.hpp"

typedef LOKI_TYPELIST_4(
    utils::testenv::with_em
    , gd::app::testenv::with_app
    , cpe::cfg::testenv::with_cfg
    , usf::logic::testenv::with_logic
    ) LogicTestBase;

class LogicTest : public testenv::fixture<LogicTestBase> {
public:
    class LogicOpMock {
    public:
        MOCK_METHOD1(execute, logic_op_exec_result_t(logic_stack_node_t node));
    };

    class CommitMock {
    public:
        MOCK_METHOD1(commit, void(logic_context_t ctx));
    };

    void SetUp();
    void TearDown();

    using Base::t_logic_executor_build;

    LogicOpMock & installOp(const char * name);
    LogicOpMock & op(const char * name);

    void set_commit(logic_context_t context, CommitMock & mock);
};

#endif
