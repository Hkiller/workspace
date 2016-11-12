#ifndef SVR_RANK_G_SVR_TEST_RANKGSVRTEST_H
#define SVR_RANK_G_SVR_TEST_RANKGSVRTEST_H
#include "gmock/gmock.h"
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "gd/app/tests-env/with_app.hpp"
#include "gd/dr_store/tests-env/with_dr_store.hpp"
#include "svr/set/stub/tests-env/with_stub.hpp"
#include "usf/logic/tests-env/with_logic.hpp"
#include "../rank_g_svr_ops.h"

typedef LOKI_TYPELIST_5(
    utils::testenv::with_em
    , cpe::cfg::testenv::with_cfg
    , gd::app::testenv::with_app
    , gd::dr_store::testenv::with_dr_store
    , svr::set::stub::testenv::with_stub
    ) RankGSvrTestBase;

class RankGSvrTest : public testenv::fixture<RankGSvrTestBase> {
public:
    void SetUp();
    void TearDown();

    void t_rank_g_svr_create(void);

    rank_g_svr_t rank_g_svr(void) { return m_rank_g_svr; }

private:
    rank_g_svr_t m_rank_g_svr;
};

#endif
