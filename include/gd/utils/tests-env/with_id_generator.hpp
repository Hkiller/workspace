#ifndef GD_UTILS_TESTENV_WITH_ID_GENERATOR_H
#define GD_UTILS_TESTENV_WITH_ID_GENERATOR_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../id_generator.h"

namespace gd { namespace utils { namespace testenv {

class with_id_generator : public ::testenv::env<> {
public:
    with_id_generator();

    void SetUp();
    void TearDown();

    gd_id_generator_t t_id_generator_create(const char * name);
    gd_id_generator_t t_id_generator(const char * name);
};

}}}

#endif
