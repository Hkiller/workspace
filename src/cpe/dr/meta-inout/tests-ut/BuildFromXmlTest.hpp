#ifndef CPE_DR_METAINOUT_TEST_BUILDFROMXMLTEST_H
#define CPE_DR_METAINOUT_TEST_BUILDFROMXMLTEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "gtest/gtest.h"
#include "cpe/utils/error_list.h"
#include "cpe/dr/dr_error.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_xml.h"
#include "../../dr_internal_types.h"

typedef LOKI_TYPELIST_1(
    utils::testenv::with_em) BuildFromXmlTestBase;

class BuildFromXmlTest : public testenv::fixture<BuildFromXmlTestBase> {
public:
    BuildFromXmlTest();
    virtual void SetUp();
    virtual void TearDown();

    LPDRMETALIB m_metaLib;
    struct mem_buffer m_buffer;

    int parseMeta(const char * def, uint8_t dft_align = 0);

    LPDRMETA meta(const char * name);
    LPDRMETAENTRY entry(const char * metaName, const char * entryName);

    int32_t address_to_pos(void * p);
};

#endif
