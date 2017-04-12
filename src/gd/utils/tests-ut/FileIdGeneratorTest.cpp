#include <sstream>
#include "cpe/pal/pal_dlfcn.h"
#include "FileIdGeneratorTest.hpp"

void FileIdGeneratorTest::SetUp() {
    Base::SetUp();

    gd_set_default_library(dlopen(NULL, RTLD_NOW));

    t_app_install_module(
        "test_id_generator", "gd_id_file_generator", NULL, NULL);
}

gd_id_generator_t FileIdGeneratorTest::id_generator(void) {
    gd_id_generator_t r = gd_id_generator_find_nc(t_app(), "test_id_generator");
    EXPECT_TRUE(r);
    return r;
}

gd_id_file_generator_t FileIdGeneratorTest::id_file_generator(void) {
    gd_id_file_generator_t r = gd_id_file_generator_find_nc(t_app(), "test_id_generator");
    EXPECT_TRUE(r);
    return r;
}

void FileIdGeneratorTest::write_in_file(const char * value, const char * id_name) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%s.id", id_name);
    t_write_to_file(t_path_make(buf), value);
}

void FileIdGeneratorTest::set_load_from_dir(void) {
    gd_id_file_generator_set_save_dir(id_file_generator(), t_dir_base());
}
