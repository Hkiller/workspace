#ifndef CPE_DR_METAINOUT_TEST_WRITETOXML_H
#define CPE_DR_METAINOUT_TEST_WRITETOXML_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "gtest/gtest.h"
#include "cpe/utils/error_list.h"
#include "cpe/dr/dr_error.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_xml.h"
#include "../../dr_internal_types.h"

typedef LOKI_TYPELIST_1(
    utils::testenv::with_em) WriteToXmlTestBase;

class WriteToXmlTest : public testenv::fixture<WriteToXmlTestBase> {
public:
    void SetUp();
    void TearDown();

    const char * writeToXml(const char * def);
};

#endif
