#ifndef CPE_DR_DATAJSON_TEST_COMPAIRETEST_H
#define CPE_DR_DATAJSON_TEST_COMPAIRETEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/cfg/cfg.h"

typedef LOKI_TYPELIST_2(
    utils::testenv::with_em
    , cpe::cfg::testenv::with_cfg) CompaireTestBase;

class CompaireTest
    : public testenv::fixture<CompaireTestBase>
{
public:
    int compaire(const char * l, const char * r, int policy = 0);
};

#endif
