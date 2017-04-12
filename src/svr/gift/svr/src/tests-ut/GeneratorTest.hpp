#ifndef SVR_GIFT_GENERATE_TEST_H
#define SVR_GIFT_GENERATE_TEST_H
#include "../gift_svr_generator.h"
#include "GiftSvrTest.hpp"

typedef ::Loki::NullType GeneratorTestBase;

class GeneratorTest : public testenv::fixture<GeneratorTestBase, GiftSvrTest> {
public:
    void SetUp();
    void TearDown();

    void t_generator_create(const char * name, const char * def);
    const char * select_prefix(void);
    gift_svr_generator_t m_generator;
};

#endif
