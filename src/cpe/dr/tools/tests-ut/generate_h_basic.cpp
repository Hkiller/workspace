#include "../generate_ops.h"
#include "GenerateTest.hpp"

TEST_F(GenerateTest, h_basic) {
    add_buffer(
        "a",
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' desc='PkgHead.desc' version='1' id='33' align='1'>"
        "	     <entry name='a1' type='int8'/>"
        "    </struct>"
        "</metalib>");

    EXPECT_STREQ(
        "#ifndef DR_GENERATED_H_net_a_INCLEDED\n"
        "#define DR_GENERATED_H_net_a_INCLEDED\n"
        "#include \"cpe/pal/pal_types.h\"\n"
        "\n"
        "\n"
        "\n"
        "#ifdef __cplusplus\n"
        "extern \"C\" {\n"
        "#endif\n"
        "\n"
        "#pragma pack(1)\n"
        "\n"
        "typedef struct _PkgHead {\n"
        "    int8_t a1;\n"
        "} PKGHEAD;\n"
        "\n"
        "#pragma pack()\n"
        "\n"
        "#ifdef __cplusplus\n"
        "}\n"
        "#endif\n"
        "\n"
        "#endif\n"
        ,
        generate_h("a"));
}
