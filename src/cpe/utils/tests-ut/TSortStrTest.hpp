#ifndef CPE_DR_TEST_TSORTSTRTEST_H
#define CPE_DR_TEST_TSORTSTRTEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tsort.h"

class TSortStrTest : public testenv::fixture<> {
public:
    virtual void SetUp();
    virtual void TearDown();

    void addDepend(const char * dep_from, const char * dep_to);
    const char * sort(void);
    int sort(const char * & r);

    tsorter_str_t m_sorter;
};

#endif
