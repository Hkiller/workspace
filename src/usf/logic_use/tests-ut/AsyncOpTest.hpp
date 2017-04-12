#ifndef USF_LOGIC_USE_TEST_ASNYCNOPTEST_H
#define USF_LOGIC_USE_TEST_ASNYCNOPTEST_H
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
    ) AsyncOpTestBase;

class AsyncOpTest : public testenv::fixture<AsyncOpTestBase> {
public:
    class OpMock {
    public:
        MOCK_METHOD1(send, logic_op_exec_result_t(logic_context_t ctx));
        MOCK_METHOD1(recv, logic_op_exec_result_t(logic_context_t ctx));
    };

    AsyncOpTest();

    void SetUp();
    void TearDown();

    OpMock m_opMock;
    logic_executor_type_t m_type;
    logic_executor_t m_executor;
    logic_context_t m_context;

    void expect_send_return(logic_op_exec_result_t rv);
    void expect_recv_return(logic_op_exec_result_t rv);

    logic_context_state_t state(void);
    int32_t rv(void);

    void execute(void);
};

#endif
