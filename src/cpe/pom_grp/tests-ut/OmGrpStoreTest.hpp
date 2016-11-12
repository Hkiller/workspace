#ifndef CPE_OM_GRP_TEST_STORE_H
#define CPE_OM_GRP_TEST_STORE_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/dr/tests-env/with_dr.hpp"
#include "cpe/pom_grp/pom_grp_store.h"
#include "cpe/pom_grp/tests-env/with_pom_grp.hpp"

typedef LOKI_TYPELIST_4(
    utils::testenv::with_em,
    cpe::cfg::testenv::with_cfg,
    cpe::dr::testenv::with_dr,
    cpe::pom_grp::testenv::with_pom_grp) OmGrpStoreTestBase;

class OmGrpStoreTest : public testenv::fixture<OmGrpStoreTestBase> {
public:
    OmGrpStoreTest();

    virtual void SetUp();
    virtual void TearDown();

    void install(const char * om_meta, const char * metalib);

    const char * str_store_meta(void);
    LPDRMETALIB store_meta(void);

    pom_grp_meta_t m_meta;
    pom_grp_store_t m_store;
    LPDRMETALIB m_input_metalib;
};

#endif
