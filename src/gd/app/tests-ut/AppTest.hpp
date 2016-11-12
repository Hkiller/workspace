#ifndef CPE_APP_TEST_APPTEST_H
#define CPE_APP_TEST_APPTEST_H
#include <string>
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "gd/app/tests-env/with_app.hpp"
#include "../app_internal_ops.h"

typedef LOKI_TYPELIST_2(
    utils::testenv::with_em,
    gd::app::testenv::with_app) AppTestBase;

class AppTest : public testenv::fixture<AppTestBase> {
public:
    virtual void SetUp(void);

    gd_app_module_t installTestModule(const char * name);
};

#endif
