#include "cpe/dr/dr_metalib_manage.h"
#include "ParseTreeTest.hpp"

#define DEF_PARSE_TYPED_TEST(__typeName, __type, __input, __expect)     \
    TEST_F(ParseTreeTest, type_ ## __typeName) {                            \
    installMeta(                                                        \
    "<metalib tagsetversion='1' name='net'  version='1'>"               \
    "    <struct name='S' version='1'>"                                 \
    "	     <entry name='a1' type='" #__typeName "'/>"                 \
    "    </struct>"                                                     \
    "</metalib>"                                                        \
        );                                                              \
                                                                        \
    struct {                                                            \
        __type a1;                                                      \
    } expect = { __expect };                                            \
                                                                        \
    ASSERT_EQ(metaSize("S"), read("{ \"a1\" : " __input "}", "S"));     \
    ASSERT_JSON_TREE_READ_RESULT(expect);                               \
}

DEF_PARSE_TYPED_TEST(int8, int8_t, "12", 12)
DEF_PARSE_TYPED_TEST(uint8, uint8_t, "12", 12)
DEF_PARSE_TYPED_TEST(int16, int16_t, "12", 12)
DEF_PARSE_TYPED_TEST(uint16, uint16_t, "12", 12)
DEF_PARSE_TYPED_TEST(int32, int32_t, "12", 12)
DEF_PARSE_TYPED_TEST(uint32, uint32_t, "12", 12)
DEF_PARSE_TYPED_TEST(char, char, "97", 'a')
DEF_PARSE_TYPED_TEST(uchar, unsigned char, "97", 'a')

TEST_F(ParseTreeTest, type_string) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1'>"
        "	     <entry name='a1' type='string' size='5'/>"
        "    </struct>"
        "</metalib>"
        );
    ASSERT_EQ(metaSize("S"), read("{ \"a1\" : \"abc\"}", "S"));

    EXPECT_STREQ("abc", (const char *)result());
}

TEST_F(ParseTreeTest, type_string_overflow) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1'>"
        "	     <entry name='a1' type='string' size='5'/>"
        "    </struct>"
        "</metalib>"
        );
    ASSERT_EQ(metaSize("S"), read("{ \"a1\" : \"abcde\"}", "S"));

    EXPECT_STREQ("abcd", (const char *)result());
}
