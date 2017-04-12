#ifndef CPE_DR_TEST_FILETEST_H
#define CPE_DR_TEST_FILETEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_file.hpp"
#include "cpe/utils/tests-env/with_em.hpp"

typedef LOKI_TYPELIST_2(
    utils::testenv::with_file
    , utils::testenv::with_em) FileTestBase;

class FileTest : public testenv::fixture<FileTestBase> {
};

#endif

