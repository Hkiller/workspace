#include "WriteTest.hpp"

TEST_F(WriteTest, data_string) {
    cfg_struct_add_string(m_root, "a", "123", cfg_replace);

    EXPECT_EQ(0, write(m_root));
    EXPECT_STREQ(
        "---\n"
        "a: '123'\n"
        "...\n"
        , result());
}

TEST_F(WriteTest, data_int32) {
    cfg_struct_add_int32(m_root, "a", 123, cfg_replace);

    EXPECT_EQ(0, write(m_root));
    EXPECT_STREQ(
        "---\n"
        "a: 123\n"
        "...\n"
        , result());
}

#define DEF_CFG_YAML_WRITE_TYPE_TESTCASE(__type, __value)    \
TEST_F(WriteTest, data_ ## __type) {                         \
    cfg_struct_add_ ##__type(                                \
        m_root, "a", __value, cfg_replace);                  \
                                                             \
    EXPECT_EQ(0, write(m_root));                             \
    EXPECT_STREQ(                                            \
        "---\n"                                              \
        "a: !" #__type " " #__value "\n"                     \
        "...\n"                                              \
    , result());                                             \
}

DEF_CFG_YAML_WRITE_TYPE_TESTCASE(int8, -123)
DEF_CFG_YAML_WRITE_TYPE_TESTCASE(uint8, 123)
DEF_CFG_YAML_WRITE_TYPE_TESTCASE(int16, -123)
DEF_CFG_YAML_WRITE_TYPE_TESTCASE(uint16, 123)
DEF_CFG_YAML_WRITE_TYPE_TESTCASE(uint32, 123)
DEF_CFG_YAML_WRITE_TYPE_TESTCASE(int64, -123)
DEF_CFG_YAML_WRITE_TYPE_TESTCASE(uint64, 123)

