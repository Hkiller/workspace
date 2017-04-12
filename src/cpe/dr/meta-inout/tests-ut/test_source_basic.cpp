#include "../dr_builder_ops.h"
#include "BuilderTest.hpp"

class SourceBasicTest : public BuilderTest {
public:
    SourceBasicTest() : m_source(NULL) {}

    virtual void SetUp(void) {
        BuilderTest::SetUp();
        m_source = dr_metalib_builder_add_file(m_builder, NULL, "a.xml");
        ASSERT_TRUE(m_source);
    }

    dr_metalib_source_t m_source;
};

TEST_F(SourceBasicTest, add_include_file_basic) {
    dr_metalib_source_t using_source =
        dr_metalib_source_add_include_file(m_source, NULL, "b.xml", dr_metalib_source_from_depend);
    ASSERT_TRUE(using_source);

    EXPECT_EQ(dr_metalib_source_type_file, dr_metalib_source_type(using_source));
    EXPECT_EQ(dr_metalib_source_format_xml, dr_metalib_source_format(using_source));
    EXPECT_EQ(dr_metalib_source_from_depend, dr_metalib_source_from(using_source));
    EXPECT_EQ(dr_metalib_source_state_not_analize, dr_metalib_source_state(using_source));

    EXPECT_STREQ("b", dr_metalib_source_name(using_source));
}

TEST_F(SourceBasicTest, add_include_file_with_name) {
    dr_metalib_source_t using_source =
        dr_metalib_source_add_include_file(m_source, "c", "b.xml", dr_metalib_source_from_depend);
    ASSERT_TRUE(using_source);

    EXPECT_STREQ("c", dr_metalib_source_name(using_source));
}

TEST_F(SourceBasicTest, it_includes_basic) {
    dr_metalib_source_t using1 =
        dr_metalib_source_add_include_file(m_source, "c", "b.xml", dr_metalib_source_from_depend);
    dr_metalib_source_t using2 =
        dr_metalib_source_add_include_file(m_source, "d", "b.xml", dr_metalib_source_from_depend);

    struct dr_metalib_source_it it;
    dr_metalib_source_includes(&it, m_source);

    EXPECT_TRUE(using1 == dr_metalib_source_next(&it));
    EXPECT_TRUE(using2 == dr_metalib_source_next(&it));
    EXPECT_TRUE(NULL == dr_metalib_source_next(&it));
}

TEST_F(SourceBasicTest, it_includes_empty) {
    struct dr_metalib_source_it it;
    dr_metalib_source_includes(&it, m_source);
    EXPECT_TRUE(NULL == dr_metalib_source_next(&it));
}

TEST_F(SourceBasicTest, it_include_by_basic) {

    dr_metalib_source_t user1 = dr_metalib_builder_add_file(m_builder, NULL, "b.xml");
    dr_metalib_source_t user2 = dr_metalib_builder_add_file(m_builder, NULL, "c.xml");

    dr_metalib_source_add_include(user1, m_source);
    dr_metalib_source_add_include(user2, m_source);

    struct dr_metalib_source_it it;
    dr_metalib_source_include_by(&it, m_source);

    EXPECT_TRUE(user1 == dr_metalib_source_next(&it));
    EXPECT_TRUE(user2 == dr_metalib_source_next(&it));
    EXPECT_TRUE(NULL == dr_metalib_source_next(&it));
}

TEST_F(SourceBasicTest, it_include_by_empty) {
    struct dr_metalib_source_it it;
    dr_metalib_source_include_by(&it, m_source);
    EXPECT_TRUE(NULL == dr_metalib_source_next(&it));
}

TEST_F(SourceBasicTest, element_it_basic) {
    struct dr_metalib_source_element_it it;

    dr_metalib_source_element_t e1 =
        dr_metalib_source_element_create(m_source, dr_metalib_source_element_type_macro, "b");
    EXPECT_TRUE(e1);
    dr_metalib_source_element_t e2 =
        dr_metalib_source_element_create(m_source, dr_metalib_source_element_type_macro, "a");
    EXPECT_TRUE(e2);

    dr_metalib_source_elements(&it, m_source);
    EXPECT_TRUE(e1 == dr_metalib_source_element_next(&it));
    EXPECT_TRUE(e2 == dr_metalib_source_element_next(&it));
    EXPECT_TRUE(NULL == dr_metalib_source_element_next(&it));
}

TEST_F(SourceBasicTest, element_it_empty) {
    struct dr_metalib_source_element_it it;
    dr_metalib_source_elements(&it, m_source);
    EXPECT_TRUE(NULL == dr_metalib_source_next(&it));
}
