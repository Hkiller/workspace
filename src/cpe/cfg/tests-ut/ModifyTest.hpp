#ifndef CPE_DR_DATAJSON_TEST_MODIFYTEST_H
#define CPE_DR_DATAJSON_TEST_MODIFYTEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/cfg/cfg.h"

typedef LOKI_TYPELIST_2(
    utils::testenv::with_em,
    cpe::cfg::testenv::with_cfg) ModifyTestBase;

class ModifyTest : public testenv::fixture<ModifyTestBase> {
public:
    ModifyTest();

    const char * modify(const char * cfg, const char * modify); 
};

#endif
