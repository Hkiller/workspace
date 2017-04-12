#include "RangeBitarryTest.hpp"

void RangeBitarryTest::SetUp(void) {
    Base::SetUp();
}

void RangeBitarryTest::TearDown(void) {
    Base::TearDown();
}

cpe_ba_t RangeBitarryTest::create_ba(const char * data) {
    return cpe_ba_create_from_string(t_tmp_allocrator(), data);
}

const char *
RangeBitarryTest::ba_to_string(cpe_ba_t ba, size_t ba_capacity) {
    return cpe_ba_to_str_create(t_tmp_allocrator(), ba, ba_capacity);
}
