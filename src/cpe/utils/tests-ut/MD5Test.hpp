#ifndef CPE_DR_TEST_MD5TEST_H
#define CPE_DR_TEST_MD5TEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/md5.h"

class MD5Test : public testenv::fixture<> {
public:
    const char * str_md5(const char * input);
    const char * to_str(cpe_md5_ctx_t ctx);
    const char * to_str(cpe_md5_value_t value);
};

#endif
