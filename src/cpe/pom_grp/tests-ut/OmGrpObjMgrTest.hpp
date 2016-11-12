#ifndef CPE_OM_GRP_TEST_MGRTEST_H
#define CPE_OM_GRP_TEST_MGRTEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/dr/tests-env/with_dr.hpp"
#include "cpe/pom_grp/tests-env/with_pom_grp.hpp"
#include "cpe/pom_grp/pom_grp_obj.h"

typedef LOKI_TYPELIST_4(
    utils::testenv::with_em,
    cpe::cfg::testenv::with_cfg,
    cpe::dr::testenv::with_dr,
    cpe::pom_grp::testenv::with_pom_grp) OmGrpObjMgrTestBase;

class OmGrpObjMgrTest : public testenv::fixture<OmGrpObjMgrTestBase> {
public:
    OmGrpObjMgrTest();

    virtual void SetUp();
    virtual void TearDown();

    void install(
        const char * om_meta,
        const char * metalib, 
        size_t capacity = 20480, uint16_t page_size = 256);

    const char * pages(pom_grp_obj_t obj);

    void reload(void);

    pom_grp_obj_mgr_t m_mgr;
};

#endif
