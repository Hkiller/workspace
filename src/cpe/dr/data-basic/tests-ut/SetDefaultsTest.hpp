#ifndef CPE_DR_DATABASIC_SETDEFAULTSTEST_H
#define CPE_DR_DATABASIC_SETDEFAULTSTEST_H
#include "DataBasicTest.hpp"

class SetDefaultsTest : public DataBasicTest {
public:
    void set_defaults(const char * metaName, int policy = 0, size_t capacity = 0);
};

#endif
