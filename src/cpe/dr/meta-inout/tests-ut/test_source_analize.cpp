#include "BuilderTest.hpp"

class SourceAnalizeTest : public BuilderTest {
};

TEST_F(SourceAnalizeTest, include_basic) {
    dr_metalib_source_t source =
        dr_metalib_builder_add_buf(
            m_builder,
            "a",
            dr_metalib_source_format_xml,
            "<metalib tagsetversion='1' name='net'  version='10'>"
            "    <struct name='PkgHead' desc='PkgHead.desc' version='1' id='33'>"
            "	     <entry name='a1' type='int8'/>"
            "    </struct>"
            "</metalib>");
    ASSERT_TRUE(source);

    dr_metalib_source_analize(source);
}
