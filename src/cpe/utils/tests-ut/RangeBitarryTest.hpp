#ifndef CPE_DR_TEST_RANGEBITARRY_H
#define CPE_DR_TEST_RANGEBITARRY_H
#include "cpe/utils/range_bitarry.h"
#include "RangeMgrTest.hpp"

class RangeBitarryTest : public testenv::fixture<Loki::NullType, RangeMgrTest> {
public:
    virtual void SetUp();
    virtual void TearDown();

    cpe_ba_t create_ba(const char * data);

    const char * ba_to_string(cpe_ba_t ba, size_t ba_capacity);
};

#endif
