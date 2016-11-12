#ifndef CPE_DR_DATAYAML_TEST_PRINTTEST_H
#define CPE_DR_DATAYAML_TEST_PRINTTEST_H
#include <string.h>
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/dr/dr_yaml.h"

typedef LOKI_TYPELIST_1(
    utils::testenv::with_em) PrintTestBase;

class PrintTest : public testenv::fixture<PrintTestBase> {
public:
    PrintTest();
    virtual void SetUp();
    virtual void TearDown();

    LPDRMETALIB m_metaLib;
    struct mem_buffer m_metaLib_buffer;

    struct mem_buffer m_buffer;
    error_list_t m_errorList;

    void installMeta(const char * data);
    int print(const void * data, size_t size, const char * typeName);
    int print_array(const void * data, size_t size, const char * typeName);
    const char * result(void);
};

#endif
