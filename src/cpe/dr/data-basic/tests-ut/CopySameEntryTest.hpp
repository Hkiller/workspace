#ifndef CPE_DR_DATABASIC_SETDEFAULTSTEST_H
#define CPE_DR_DATABASIC_SETDEFAULTSTEST_H
#include "DataBasicTest.hpp"

class CopySameEntryTest : public DataBasicTest {
public:
    int copy(const char * desMeta, const char * srcMeta, int policy = 0, size_t capacity = 0);
    void copy_part(const char * desMeta, const char * srcMeta, const char * columsn, int policy = 0, size_t capacity = 0);
};

#endif
