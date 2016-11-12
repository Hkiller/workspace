#ifndef CPE_DR_TOOLS_TEST_GENERATETEST_H
#define CPE_DR_TOOLS_TEST_GENERATETEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/dr/dr_metalib_builder.h"
#include "../generate_ops.h"

typedef LOKI_TYPELIST_1(
    utils::testenv::with_em) GenerateTestBase;

class GenerateTest : public testenv::fixture<GenerateTestBase> {
public:
    GenerateTest();
    virtual void SetUp();
    virtual void TearDown();

    dr_metalib_source_t add_buffer(const char * name, const char * data);
    dr_metalib_source_t source(const char * name);
    void prepare_ctx(void);

    const char * generate_h(const char * name);
    const char * generate_lib_c(const char * arg_name);

    cpe_dr_generate_ctx m_ctx;
};

#endif
