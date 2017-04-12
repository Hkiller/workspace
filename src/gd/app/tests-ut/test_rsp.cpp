#include "cpe/dp/dp_responser.h"
#include "cpe/dp/dp_manage.h"
#include "AppTest.hpp"

class RspTest : public AppTest {
public:
    virtual void SetUp() {
        AppTest::SetUp();
        ASSERT_TRUE(installTestModule("module1"));
    }
};

extern "C" {
int rsp_processor_1(dp_req_t req, void * ctx, error_monitor_t em) { return 0; }
int rsp_processor_1_init(dp_rsp_t rsp, gd_app_context_t context, gd_app_module_type_t module, cfg_t cfg) { 
    dp_rsp_set_processor(rsp, rsp_processor_1, NULL);
    return 0;
}
int rsp_init_no_processor(dp_rsp_t rsp, gd_app_context_t context, gd_app_module_type_t module, cfg_t cfg) { return 0; }
int rsp_init_fail(dp_rsp_t rsp, gd_app_context_t context, gd_app_module_type_t module, cfg_t cfg) { return -1; }
}

TEST_F(RspTest, rsp_load_by_processor) {
    EXPECT_EQ(
        0,
        t_app_install_rsps(
            "module1",
            "rsps:\n"
            "  - name: rsp1\n"
            "    respons-to: []\n"
            "    processor: rsp_processor_1\n"
            ));

    dp_rsp_t rsp1 = dp_rsp_find_by_name(t_dp(), "rsp1");
    ASSERT_TRUE(rsp1);
    EXPECT_STREQ("rsp1", dp_rsp_name(rsp1));
}

TEST_F(RspTest, rsp_load_by_processor_not_exist) {
    EXPECT_EQ(
        -1,
        t_app_install_rsps(
            "module1",
            "rsps:\n"
            "  - name: rsp1\n"
            "    respons-to: []\n"
            "    processor: rsp_processor_not_exist\n"
            ));

    ASSERT_TRUE(NULL == dp_rsp_find_by_name(t_dp(), "rsp1"));
}

TEST_F(RspTest, rsp_load_by_init) {
    EXPECT_EQ(
        0,
        t_app_install_rsps(
            "module1",
            "rsps:\n"
            "  - name: rsp1\n"
            "    respons-to: []\n"
            "    init: rsp_processor_1_init\n"
            ));

    dp_rsp_t rsp1 = dp_rsp_find_by_name(t_dp(), "rsp1");
    ASSERT_TRUE(rsp1);
    EXPECT_STREQ("rsp1", dp_rsp_name(rsp1));
}

TEST_F(RspTest, rsp_load_by_init_not_exist) {
    EXPECT_EQ(
        -1,
        t_app_install_rsps(
            "module1",
            "rsps:\n"
            "  - name: rsp1\n"
            "    respons-to: []\n"
            "    init: rsp_not_exist_init\n"
            ));

    ASSERT_TRUE(NULL == dp_rsp_find_by_name(t_dp(), "rsp1"));
}

TEST_F(RspTest, rsp_load_by_init_fail) {
    EXPECT_EQ(
        -1,
        t_app_install_rsps(
            "module1",
            "rsps:\n"
            "  - name: rsp1\n"
            "    respons-to: []\n"
            "    init: rsp_init_fail\n"
            ));

    ASSERT_TRUE(NULL == dp_rsp_find_by_name(t_dp(), "rsp1"));
}

TEST_F(RspTest, rsp_load_by_init_no_processor) {
    EXPECT_EQ(
        -1,
        t_app_install_rsps(
            "module1",
            "rsps:\n"
            "  - name: rsp1\n"
            "    respons-to: []\n"
            "    init: rsp_init_no_processor\n"
            ));

    ASSERT_TRUE(NULL == dp_rsp_find_by_name(t_dp(), "rsp1"));
}

TEST_F(RspTest, rsp_load_both_cfg) {
    EXPECT_EQ(
        -1,
        t_app_install_rsps(
            "module1",
            "rsps:\n"
            "  - name: rsp1\n"
            "    respons-to: []\n"
            "    processor: rsp_processor_1\n"
            "    init: rsp_processor_1_init\n"
            ));

    ASSERT_TRUE(NULL == dp_rsp_find_by_name(t_dp(), "rsp1"));
}

TEST_F(RspTest, rsp_load_no_name) {
    EXPECT_EQ(
        -1,
        t_app_install_rsps(
            "module1",
            "rsps:\n"
            "  - rsp-to: 1\n"
            "    processor: rsp_processor_1\n"
            ));

    EXPECT_TRUE(NULL == dp_rsp_find_first_by_numeric(t_dp(), 1));
}

TEST_F(RspTest, rsp_load_no_processor) {
    EXPECT_EQ(
        -1,
        t_app_install_rsps(
            "module1",
            "rsps:\n"
            "  - name: rsp1\n"
            "    respons-to: [1, ack]\n"
            "    processor: rsp_processor_not_exist\n"
            ));

    EXPECT_TRUE(NULL == dp_rsp_find_first_by_numeric(t_dp(), 1));
}

TEST_F(RspTest, rsp_load_single_binding) {
    EXPECT_EQ(
        0,
        t_app_install_rsps(
            "module1",
            "rsps:\n"
            "  - name: rsp1\n"
            "    respons-to: 1\n"
            "    processor: rsp_processor_1\n"
            ));

    dp_rsp_t rsp1 = dp_rsp_find_by_name(t_dp(), "rsp1");
    ASSERT_TRUE(rsp1);

    EXPECT_TRUE(rsp1 == dp_rsp_find_first_by_numeric(t_dp(), 1));
}

TEST_F(RspTest, rsp_load_multi_binding) {
    EXPECT_EQ(
        0,
        t_app_install_rsps(
            "module1",
            "rsps:\n"
            "  - name: rsp1\n"
            "    respons-to: [1, ack]\n"
            "    processor: rsp_processor_1\n"
            ));

    dp_rsp_t rsp1 = dp_rsp_find_by_name(t_dp(), "rsp1");
    ASSERT_TRUE(rsp1);

    EXPECT_TRUE(rsp1 == dp_rsp_find_first_by_numeric(t_dp(), 1));
    EXPECT_TRUE(rsp1 == dp_rsp_find_first_by_string(t_dp(), "ack"));
}

TEST_F(RspTest, rsp_load_no_binding) {
    EXPECT_EQ(
        0,
        t_app_install_rsps(
            "module1",
            "rsps:\n"
            "  - name: rsp1\n"
            "    respons-to: []\n"
            "    processor: rsp_processor_1\n"
            ));

    dp_rsp_t rsp1 = dp_rsp_find_by_name(t_dp(), "rsp1");
    ASSERT_TRUE(rsp1);
}

extern "C"
int rsp_default_with_init_init(dp_rsp_t rsp, gd_app_context_t context, gd_app_module_type_t module, cfg_t cfg) { 
    dp_rsp_set_processor(rsp, rsp_processor_1, NULL);
    return 0;
}

TEST_F(RspTest, rsp_load_default_with_init) {
    EXPECT_EQ(
        0,
        t_app_install_rsps(
            "module1",
            "rsps:\n"
            "  - name: default_with_init\n"
            "    respons-to: []\n"
            ));

    ASSERT_TRUE(NULL != dp_rsp_find_by_name(t_dp(), "default_with_init"));
}

extern "C"
int rsp_default_with_processor(dp_req_t req, void * ctx, error_monitor_t em) { return 0; }

TEST_F(RspTest, rsp_load_default_with_processor) {
    EXPECT_EQ(
        0,
        t_app_install_rsps(
            "module1",
            "rsps:\n"
            "  - name: default_with_processor\n"
            "    respons-to: []\n"
            ));

    ASSERT_TRUE(NULL != dp_rsp_find_by_name(t_dp(), "default_with_processor"));
}

TEST_F(RspTest, rsp_load_default_no) {
    EXPECT_EQ(
        -1,
        t_app_install_rsps(
            "module1",
            "rsps:\n"
            "  - name: default_no\n"
            "    respons-to: []\n"
            ));

    ASSERT_TRUE(NULL == dp_rsp_find_by_name(t_dp(), "default_no"));
}

TEST_F(RspTest, rsp_auto_free) {
    EXPECT_EQ(
        0,
        t_app_install_rsps(
            "module1",
            "rsps:\n"
            "  - name: rsp1\n"
            "    respons-to: []\n"
            "    processor: rsp_processor_1\n"
            ));

    ASSERT_TRUE(NULL != dp_rsp_find_by_name(t_dp(), "rsp1"));

    gd_app_uninstall_module(t_app(), "module1");
    ASSERT_TRUE(NULL == t_app_find_module("module1"));
    ASSERT_TRUE(NULL == dp_rsp_find_by_name(t_dp(), "rsp1"));
}

TEST_F(RspTest, rsp_load_from_multi_level) {
    EXPECT_EQ(
        0,
        t_app_install_rsps(
            "module1",
            "rsps:\n"
            "    rsp-group-level-1:\n"
            "        rsp-group-level-2:\n"
            "            - name: rsp1\n"
            "              respons-to: []\n"
            "              processor: rsp_processor_1\n"
            ));

    ASSERT_TRUE(NULL != dp_rsp_find_by_name(t_dp(), "rsp1"));
}
