#include "cpe/pal/pal_dlfcn.h"
#include "AppTest.hpp"

extern "C" {
int test_module_global_init() { return 0; }
void test_module_global_fini() {}
}

void AppTest::SetUp() {
    Base::SetUp();
    gd_set_default_library(dlopen(NULL, RTLD_NOW));
}

gd_app_module_t
AppTest::installTestModule(const char * name) {
    return t_app_install_module(name, "test_module", NULL);
}
