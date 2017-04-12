#ifndef USF_BPG_PKG_TEST_BPGPKGTEST_H
#define USF_BPG_PKG_TEST_BPGPKGTEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/dp/dp_request.h"
#include "gd/app/tests-env/with_app.hpp"
#include "gd/dr_cvt/tests-env/with_dr_cvt.hpp"
#include "gd/dr_store/tests-env/with_dr_store.hpp"
#include "usf/bpg_pkg/tests-env/with_bpg_pkg.hpp"

typedef LOKI_TYPELIST_6(
    utils::testenv::with_em
    , cpe::cfg::testenv::with_cfg
    , gd::app::testenv::with_app
    , gd::dr_cvt::testenv::with_dr_cvt
    , gd::dr_store::testenv::with_dr_store
    , usf::bpg::testenv::with_bpg_pkg
    ) BpgPkgTestBase;

class BpgPkgTest : public testenv::fixture<BpgPkgTestBase> {
public:
    void SetUp();
    void TearDown();
};

#endif
