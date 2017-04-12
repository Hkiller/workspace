#ifndef CPE_DR_DATABASIC_SETDEFAULTSTEST_H
#define CPE_DR_DATABASIC_SETDEFAULTSTEST_H
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "DataBasicTest.hpp"

typedef LOKI_TYPELIST_1(
    cpe::cfg::testenv::with_cfg) DataCvtTestBase;

class DataCvtTest : public testenv::fixture<DataCvtTestBase, DataBasicTest> {
public:
    void cvt(const char * desMeta, const char * srcMeta, const char * cfg, size_t capacity = 0);
};

#endif
