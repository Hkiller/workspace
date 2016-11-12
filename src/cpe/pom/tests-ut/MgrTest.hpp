#ifndef CPE_OM_TEST_MGRTEST_H
#define CPE_OM_TEST_MGRTEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/pom/pom_error.h"
#include "cpe/pom/pom_manage.h"
#include "cpe/pom/pom_class.h"
#include "cpe/pom/pom_object.h"

typedef LOKI_TYPELIST_1(
    utils::testenv::with_em) MgrTestBase;

class MgrTest : public testenv::fixture<MgrTestBase> {
public:
    MgrTest();

    virtual void SetUp();
    virtual void TearDown();

    pom_class_id_t addClass(const char * className, size_t object_size);
    pom_oid_t obj_alloc(cpe_hash_string_t className);

    static pom_buffer_id_t buf_alloc(size_t size, void * context);

    pom_mgr_t m_omm;
};

#endif
