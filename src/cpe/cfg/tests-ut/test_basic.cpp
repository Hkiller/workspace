#include "CfgTest.hpp"

TEST_F(CfgTest, name_of_root) {
    EXPECT_STREQ("", cfg_name(m_root));
}

TEST_F(CfgTest, name_of_struct_item) {
    EXPECT_STREQ(
        "aa",
        cfg_name(
            cfg_struct_add_struct(m_root, "aa", cfg_replace)));
}

TEST_F(CfgTest, name_of_seq_item) {
    EXPECT_STREQ(
        "aa",
        cfg_name(
            cfg_seq_add_int8(
                cfg_struct_add_seq(m_root, "aa", cfg_replace)
                , 3)));
}

TEST_F(CfgTest, data_of_struct) {
    EXPECT_TRUE(NULL == cfg_data(m_root));
}

TEST_F(CfgTest, data_of_sequence) {
    EXPECT_TRUE(NULL == cfg_data(cfg_struct_add_seq(m_root, "a", cfg_replace)));
}

TEST_F(CfgTest, is_value_of_struct) {
    EXPECT_FALSE(cfg_is_value(m_root));
}

TEST_F(CfgTest, is_value_of_sequence) {
    EXPECT_FALSE(cfg_is_value(cfg_struct_add_seq(m_root, "a", cfg_replace)));
}

TEST_F(CfgTest, is_value_of_int) {
    EXPECT_TRUE(cfg_is_value(cfg_struct_add_int8(m_root, "a", 0, cfg_replace)));
}

