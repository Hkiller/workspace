#ifndef USF_LOGIC_TEST_LOGICTEST_H
#define USF_LOGIC_TEST_LOGICTEST_H
#include <string>
#include "gmock/gmock.h"
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "gd/app/tests-env/with_app.hpp"
#include "usf/logic/tests-env/with_logic.hpp"
#include "usf/bpg_rsp/tests-env/with_bpg_rsp.hpp"

typedef LOKI_TYPELIST_5(
    utils::testenv::with_em
    , gd::app::testenv::with_app
    , cpe::cfg::testenv::with_cfg
    , usf::logic::testenv::with_logic
    , usf::bpg::testenv::with_bpg_rsp
    ) BpgTestBase;

class BpgTest : public testenv::fixture<BpgTestBase> {
public:
    void SetUp();
    void TearDown();
};

#endif
