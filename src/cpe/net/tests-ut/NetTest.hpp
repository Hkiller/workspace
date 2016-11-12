#ifndef GD_NET_TEST_NETTEST_H
#define GD_NET_TEST_NETTEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/net/tests-env/with_net.hpp"

typedef LOKI_TYPELIST_2(
    utils::testenv::with_em
    , cpe::net::testenv::with_net) NetTestBase;

class NetTest : public testenv::fixture<NetTestBase> {
public:
    NetTest();

    virtual void SetUp(void);
    virtual void TearDown(void);

private:
};

#endif
