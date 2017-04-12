#ifndef CPEPP_CFG_NODETEST_H
#define CPEPP_CFG_NODETEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpepp/cfg/Node.hpp"

class NodeTest : public testenv::fixture<> {
public:
    NodeTest();
    virtual void SetUp();
    virtual void TearDown();

    void install(const char * input);

    Cpe::Cfg::Node & root(void) { return *((Cpe::Cfg::Node *)_cfg); }
private:
    cfg_t _cfg;
};

#endif
