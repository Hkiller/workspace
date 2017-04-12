#ifndef USF_LOGIC_TEST_RUNTEST_H
#define USF_LOGIC_TEST_RUNTEST_H
#include "LogicTest.hpp"

class RunTest : public LogicTest {
public:
    RunTest();

    virtual void SetUp();
    virtual void TearDown();

    void expect_create_require(LogicOpMock & op, logic_op_exec_result_t rv);
    void expect_return(LogicOpMock & op, logic_op_exec_result_t rv);
    void expect_commit(void);

    void execute(const char * data);
    void execute_again(void);
    void set_execute_immediately(void);

    void cancel(void);
    void timeout(void);

    logic_context_state_t state(void);
    int32_t rv(void);

    CommitMock m_commitMock;
    logic_context_t m_context;
    logic_executor_t m_executor;
};

#endif
