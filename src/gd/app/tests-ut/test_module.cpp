#include "AppTest.hpp"

class ModuleTest : public AppTest {
};

TEST_F(ModuleTest, create_basic) {
    gd_app_module_t module = installTestModule("a");
    ASSERT_TRUE(module != NULL);

    EXPECT_STREQ("a", gd_app_module_name(module));
    EXPECT_STREQ("test_module", gd_app_module_type_name(module));
    EXPECT_TRUE(NULL == gd_app_module_lib(module));
}

TEST_F(ModuleTest, create_no_init) {
    EXPECT_TRUE(
        NULL == t_app_install_module("test_module_not_exist", NULL, NULL));
}

TEST_F(ModuleTest, find_basic) {
    gd_app_module_t module = installTestModule("b");

    EXPECT_TRUE(module == gd_app_find_module(t_app(), "b"));
}

TEST_F(ModuleTest, find_multi) {
    gd_app_module_t module_a = installTestModule("a");
    gd_app_module_t module_b = installTestModule("b");

    EXPECT_TRUE(module_a == gd_app_find_module(t_app(), "a"));
    EXPECT_TRUE(module_b == gd_app_find_module(t_app(), "b"));
}

TEST_F(ModuleTest, find_not_exist) {
    EXPECT_TRUE(NULL == gd_app_find_module(t_app(), "not-exist-module"));
}

TEST_F(ModuleTest, uninstall_basic) {
    installTestModule("a");

    EXPECT_EQ(0, t_app_uninstall_module("a"));
    EXPECT_TRUE(NULL == gd_app_find_module(t_app(), "a"));
}

TEST_F(ModuleTest, uninstall_first) {
    installTestModule("a");
    gd_app_module_t module_b = installTestModule("b");
    gd_app_module_t module_c = installTestModule("c");

    EXPECT_EQ(0, t_app_uninstall_module("a"));

    EXPECT_TRUE(NULL == gd_app_find_module(t_app(), "a"));
    EXPECT_TRUE(module_b == gd_app_find_module(t_app(), "b"));
    EXPECT_TRUE(module_c == gd_app_find_module(t_app(), "c"));
}

TEST_F(ModuleTest, uninstall_last) {
    gd_app_module_t module_a = installTestModule("a");
    gd_app_module_t module_b = installTestModule("b");
    installTestModule("c");

    EXPECT_EQ(0, t_app_uninstall_module("c"));

    EXPECT_TRUE(module_a == gd_app_find_module(t_app(), "a"));
    EXPECT_TRUE(module_b == gd_app_find_module(t_app(), "b"));
    EXPECT_TRUE(NULL == gd_app_find_module(t_app(), "c"));
}

TEST_F(ModuleTest, app_free) {
    installTestModule("a");
    installTestModule("b");
    installTestModule("c");
    EXPECT_TRUE(gd_app_module_type_find("test_module"));

    t_app_free();

    EXPECT_TRUE(NULL == gd_app_module_type_find("test_module"));
}

