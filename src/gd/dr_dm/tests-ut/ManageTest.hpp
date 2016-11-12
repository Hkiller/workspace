#ifndef GDPP_DR_DM_TEST_MANAGE_H
#define GDPP_DR_DM_TEST_MANAGE_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "gd/app/tests-env/with_app.hpp"
#include "gd/utils/tests-env/with_id_generator.hpp"
#include "gd/dr_store/tests-env/with_dr_store.hpp"
#include "gd/dr_dm/tests-env/with_dr_dm.hpp"

typedef LOKI_TYPELIST_5(
    utils::testenv::with_em
    , gd::app::testenv::with_app
    , gd::utils::testenv::with_id_generator
    , gd::dr_store::testenv::with_dr_store
    , gd::dr_dm::testenv::with_dr_dm) ManageTestBase;

class ManageTest : public testenv::fixture<ManageTestBase> {
public:
    ManageTest();
    virtual void SetUp();

    void setMetaLib(const char * name);
    void setMeta(const char * name);

    dr_dm_manage_t m_manage;
};

#endif
