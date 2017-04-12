#ifndef USF_LOGIC_USE_TEST_DYNDATATEST_H
#define USF_LOGIC_USE_TEST_DYNDATATEST_H
#include "gmock/gmock.h"
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/dr/tests-env/with_dr.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "gd/app/tests-env/with_app.hpp"
#include "usf/logic/tests-env/with_logic.hpp"

typedef LOKI_TYPELIST_4(
    utils::testenv::with_em
    , cpe::dr::testenv::with_dr
    , gd::app::testenv::with_app
    , usf::logic::testenv::with_logic
    ) DynDataTestBase;

class DynDataTest : public testenv::fixture<DynDataTestBase> {
public:
    DynDataTest();

    void SetUp();
    void TearDown();

    logic_context_t m_context;
};

#endif
