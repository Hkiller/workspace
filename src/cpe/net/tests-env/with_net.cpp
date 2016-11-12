#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/net/tests-env/with_net.hpp"

namespace cpe { namespace net { namespace testenv {

with_net::with_net()
    : m_net(NULL)
{
}

void with_net::SetUp() {
    Base::SetUp();

    utils::testenv::with_em * with_em = tryEnvOf<utils::testenv::with_em>();

    m_net = net_mgr_create(t_allocrator(), with_em ? with_em->t_em() : NULL);

    ASSERT_TRUE(m_net);
}

void with_net::TearDown() {
    if (m_net) {
        net_mgr_free(m_net);
        m_net = NULL;
    }

    Base::TearDown();
}

net_mgr_t
with_net::t_net() {
    return m_net;
}

void with_net::t_net_break() {
    net_mgr_break(m_net);
}

void with_net::t_net_tick() {
    net_mgr_tick(m_net);
}

void with_net::t_net_run(net_run_tick_fun_t tick_fun, void * tick_ctx, int64_t span) {
    net_mgr_run(m_net, span, tick_fun, tick_ctx);
}

}}}
