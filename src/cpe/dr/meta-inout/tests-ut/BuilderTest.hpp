#ifndef CPE_DR_METAINOUT_TEST_BUILDERTEST_H
#define CPE_DR_METAINOUT_TEST_BUILDERTEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/dr/dr_metalib_builder.h"

typedef LOKI_TYPELIST_1(
    utils::testenv::with_em) BuilderTestBase;

class BuilderTest : public testenv::fixture<BuilderTestBase> {
public:
    BuilderTest();
    virtual void SetUp();
    virtual void TearDown();

    dr_metalib_builder_t m_builder;
};

#endif
