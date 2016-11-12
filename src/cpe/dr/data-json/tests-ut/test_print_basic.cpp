#include "PrintTest.hpp"

#define DEF_PRINT_TYPED_TEST(__typeName, __type, __input, __expect)     \
    TEST_F(PrintTest, print_ ## __typeName ) {                        \
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
    } input = { __input };                                              \
                                                                        \
    EXPECT_GT(print(&input, sizeof(input), "S"), 0);                    \
    EXPECT_STREQ("{\"a1\":" __expect "}", result());                    \
}

DEF_PRINT_TYPED_TEST(int8, int8_t, 12, "12")
DEF_PRINT_TYPED_TEST(uint8, uint8_t, 12, "12")
DEF_PRINT_TYPED_TEST(int16, int16_t, 12, "12")
DEF_PRINT_TYPED_TEST(uint16, uint16_t, 12, "12")
DEF_PRINT_TYPED_TEST(int32, int32_t, 12, "12")
DEF_PRINT_TYPED_TEST(uint32, uint32_t, 12, "12")
DEF_PRINT_TYPED_TEST(char, char, 'a', "97")
DEF_PRINT_TYPED_TEST(uchar, unsigned char, 'a', "97")

TEST_F(PrintTest, print_string) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1'>"
        "	     <entry name='a1' type='string' size='5'/>"
        "    </struct>"
        "</metalib>"
        );

    struct {
        char a1[5];
    } input = { "abcd"  };

    EXPECT_GT(print(&input, sizeof(input), "S"), 0);
    EXPECT_STREQ("{\"a1\":\"abcd\"}", result());
}

