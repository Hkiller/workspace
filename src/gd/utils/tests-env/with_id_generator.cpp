#include "gd/utils/id_generator_file.h"
#include "gd/app/tests-env/with_app.hpp"
#include "gd/utils/tests-env/with_id_generator.hpp"

namespace gd { namespace utils { namespace testenv {

with_id_generator::with_id_generator() {
}

void with_id_generator::SetUp() {
    Base::SetUp();
}

void with_id_generator::TearDown() {
    Base::TearDown();
}

gd_id_generator_t
with_id_generator::t_id_generator(const char * name) {
    gd_id_generator_t r = gd_id_generator_find_nc(
        envOf<gd::app::testenv::with_app>().t_app(),
        name);
    EXPECT_TRUE(r) << "id_generator " << name << " not exist!";
    return r;
}

gd_id_generator_t
with_id_generator::t_id_generator_create(const char * name) {
    gd_id_generator_t r =
        (gd_id_generator_t)gd_id_file_generator_create(
            envOf<gd::app::testenv::with_app>().t_app(), name, NULL, NULL);
    EXPECT_TRUE(r) << "id_generator " << name << " create fail!";
    if (r == NULL) return NULL;

    return r;
}

}}}
