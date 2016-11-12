#include "ReadTest.hpp"

#define DEF_CFG_YAML_READ_TYPE_TESTCASE_EX(                 \
    __fun,                                                  \
    __input_type, __input_value,                            \
    __output_type, __output_value)                          \
    TEST_F(ReadTest, struct_data_ ## __fun) {               \
    EXPECT_EQ(                                              \
        0, read(                                            \
            "a: " __input_type " " __input_value "\n"    \
            ));                                             \
                                                            \
    EXPECT_STREQ(                                           \
        "---\n"                                             \
        "a: " __output_type __output_value "\n"            \
        "...\n"                                             \
        , result());                                        \
}                                                           \
TEST_F(ReadTest, seq_data_ ## __fun) {                      \
    EXPECT_EQ(                                              \
        0, read(                                            \
            "- " __input_type " " __input_value "\n"       \
            ));                                             \
                                                            \
    EXPECT_STREQ(                                           \
        "---\n"                                             \
        "'':\n"                                             \
        "- " __output_type __output_value "\n"             \
        "...\n"                                             \
        , result());                                        \
}

#define DEF_CFG_YAML_READ_TYPE_TESTCASE(__type, __value) \
    DEF_CFG_YAML_READ_TYPE_TESTCASE_EX(__type, "!"#__type, #__value, "!"#__type" ", #__value)

DEF_CFG_YAML_READ_TYPE_TESTCASE(int8, -123)
DEF_CFG_YAML_READ_TYPE_TESTCASE(uint8, 123)
DEF_CFG_YAML_READ_TYPE_TESTCASE(int16, -123)
DEF_CFG_YAML_READ_TYPE_TESTCASE(uint16, 123)
DEF_CFG_YAML_READ_TYPE_TESTCASE_EX(int32, "!int32", "-123", "", "-123")
DEF_CFG_YAML_READ_TYPE_TESTCASE(uint32, 123)
DEF_CFG_YAML_READ_TYPE_TESTCASE(int64, -123)
DEF_CFG_YAML_READ_TYPE_TESTCASE(uint64, 123)
DEF_CFG_YAML_READ_TYPE_TESTCASE_EX(bool_true, "!!bool", "true", "", "1")
DEF_CFG_YAML_READ_TYPE_TESTCASE_EX(bool_false, "!!bool", "false", "", "0")
DEF_CFG_YAML_READ_TYPE_TESTCASE_EX(bool_t, "!!bool", "y", "", "1")

DEF_CFG_YAML_READ_TYPE_TESTCASE_EX(bool_f, "!!bool", "n", "", "0")
DEF_CFG_YAML_READ_TYPE_TESTCASE_EX(bool_no_tag, "", "true", "", "'true'")
DEF_CFG_YAML_READ_TYPE_TESTCASE_EX(int, "!!int", "12", "", "12")
DEF_CFG_YAML_READ_TYPE_TESTCASE_EX(force_string_by_tag, "!!str", "-123", "", "'-123'")
DEF_CFG_YAML_READ_TYPE_TESTCASE_EX(force_string_by_quart1, "", "'-123'", "", "'-123'")
DEF_CFG_YAML_READ_TYPE_TESTCASE_EX(force_string_by_quart2, "", "\"-123\"", "", "'-123'")
DEF_CFG_YAML_READ_TYPE_TESTCASE_EX(float_basic, "", "3.2", "", "3.200000")
