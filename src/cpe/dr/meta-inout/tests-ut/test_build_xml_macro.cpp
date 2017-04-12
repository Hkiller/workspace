#include "BuildFromXmlTest.hpp"

class BuildFromXmlMacroTest : public BuildFromXmlTest {
};

TEST_F(BuildFromXmlMacroTest, macro_num) {
    parseMeta(
        "<?xml version='1.0' standalone='yes' ?>"
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <macro name='VERSION' value='100' desc='VERSION.desc'/>"
        "    <macro name='MAX_BODY_LEN' value='32000' />"
        "</metalib>"
        );

    ASSERT_EQ(
        2,
        dr_lib_macro_num(m_metaLib));
}

TEST_F(BuildFromXmlMacroTest, macro_order) {
    parseMeta(
        "<?xml version='1.0' standalone='yes' ?>"
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <macro name='VERSION' value='100' desc='VERSION.desc'/>"
        "    <macro name='MAX_BODY_LEN' value='32000' />"
        "</metalib>"
        );

    LPDRMACRO macro0 = dr_lib_macro_at(m_metaLib, 0);
    ASSERT_TRUE(macro0);
    LPDRMACRO macro1 = dr_lib_macro_at(m_metaLib, 1);
    ASSERT_TRUE(macro1);

    EXPECT_STREQ("VERSION", dr_macro_name(m_metaLib, macro0));
    EXPECT_STREQ("MAX_BODY_LEN", dr_macro_name(m_metaLib, macro1));
}

TEST_F(BuildFromXmlMacroTest, macro_data) {
    parseMeta(
        "<?xml version='1.0' standalone='yes' ?>"
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <macro name='VERSION' value='100' desc='VERSION.desc'/>"
        "</metalib>"
        );


    LPDRMACRO macro0 = dr_lib_macro_at(m_metaLib, 0);
    ASSERT_TRUE(macro0);
    EXPECT_STREQ("VERSION.desc", dr_macro_desc(m_metaLib, macro0));

    ASSERT_EQ(100, dr_macro_value(macro0));
}

TEST_F(BuildFromXmlMacroTest, macro_no_value) {
    parseMeta(
        "<?xml version='1.0' standalone='yes' ?>"
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <macro name='VERSION'/>"
        "</metalib>"
        );

    ASSERT_EQ(0, dr_lib_macro_num(m_metaLib));
    ASSERT_TRUE(t_em_have_errno(CPE_DR_ERROR_MACRO_NO_VALUE));
}

TEST_F(BuildFromXmlMacroTest, macro_no_name) {
    parseMeta(
        "<?xml version='1.0' standalone='yes' ?>"
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <macro value='100'/>"
        "</metalib>"
        );

    ASSERT_EQ(0, dr_lib_macro_num(m_metaLib));
    ASSERT_TRUE(t_em_have_errno(CPE_DR_ERROR_MACRO_NO_NAME_ATTR));
}
