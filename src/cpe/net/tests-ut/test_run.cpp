#include "NetTest.hpp"

class RunTest : public NetTest {
};

void run_test_example_accept_fun(net_listener_t listener, net_ep_t ep, void * ctx) {
}

void run_test_connector_state_monitor(net_connector_t connector, void * ctx) {
    RunTest * runTest = (RunTest *)ctx;

    net_connector_state_t state = net_connector_state(connector);

    if (state == net_connector_state_error
        || state == net_connector_state_connected)
    {
        runTest->t_net_break();
    }
}

TEST_F(RunTest, run) {
    net_listener_t listener =
        net_listener_create(
            t_net(),
            "test-listener",
            "",
            0,
            5,
            run_test_example_accept_fun,
            this);
    ASSERT_TRUE(listener);

    net_connector_t connector =
        net_connector_create_with_ep(
            t_net(),
            "test-connector",
            "127.0.0.1",
            net_listener_using_port(listener));
    ASSERT_TRUE(connector);

    net_connector_add_monitor(
        connector, run_test_connector_state_monitor, this);

    net_connector_enable(connector);

    t_net_run();
}
