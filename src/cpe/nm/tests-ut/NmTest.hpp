#ifndef CPE_NM_TEST_NMTEST_H
#define CPE_NM_TEST_NMTEST_H
#include <string>
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/nm/tests-env/with_nm.hpp"

typedef LOKI_TYPELIST_2(
    utils::testenv::with_em,
    gd::nm::testenv::with_nm) NmTestBase;

class NmTest : public testenv::fixture<NmTestBase> {
public:
};

#endif
