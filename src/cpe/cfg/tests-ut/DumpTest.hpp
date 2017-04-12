#ifndef CPE_DR_DATAJSON_TEST_DUMPTEST_H
#define CPE_DR_DATAJSON_TEST_DUMPTEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/cfg/cfg.h"

class DumpTest
    : public testenv::fixture<LOKI_TYPELIST_1(cpe::cfg::testenv::with_cfg)>
{
public:
    const char * dump(const char * input);
    const char * dump_inline(const char * input);
};

#endif
