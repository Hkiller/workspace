#ifndef CPE_DR_METAINOUT_TEST_BUILDTEST_H
#define CPE_DR_METAINOUT_TEST_BUILDTEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "gtest/gtest.h"
#include "cpe/utils/error_list.h"
#include "cpe/dr/dr_error.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_build.h"
#include "../dr_inbuild_types.h"

typedef LOKI_TYPELIST_1(
    utils::testenv::with_em) BuildTestBase;

class BuildTest : public testenv::fixture<BuildTestBase> {
public:
    BuildTest();
    virtual void SetUp();
    virtual void TearDown();

    LPDRMETALIB m_metaLib;
    struct mem_buffer m_buffer;

    int build(void);

    LPDRMETA meta(const char * name);
    LPDRMETAENTRY entry(const char * metaName, const char * entryName);

    DRInBuildMetaLib * m_builder;
};

#endif
