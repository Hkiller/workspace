#ifndef CPE_AOM_TEST_AOMOBJMANAGERTEST_H
#define CPE_AOM_TEST_AOMOBJMANAGERTEST_H
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/dr/tests-env/with_dr.hpp"
#include "cpe/aom/aom_obj_mgr.h"

typedef LOKI_TYPELIST_2(
    utils::testenv::with_em
    , cpe::dr::testenv::with_dr) AomObjManagerTestBase;

class AomObjManagerTest : public testenv::fixture<AomObjManagerTestBase> {
public:
    virtual void SetUp();
    virtual void TearDown();

    void create_aom_obj_mgr(const char * meta, uint32_t record_count);

    aom_obj_mgr_t m_obj_mgr;
};

#endif
