#include "BuilderTest.hpp"

class BuilderBasicTest : public BuilderTest {
};

TEST_F(BuilderBasicTest, add_file_basic) {
    dr_metalib_source_t source = dr_metalib_builder_add_file(m_builder, NULL, "b/a.xml");
    ASSERT_TRUE(source);

    EXPECT_EQ(dr_metalib_source_type_file, dr_metalib_source_type(source));
    EXPECT_EQ(dr_metalib_source_format_xml, dr_metalib_source_format(source));
    EXPECT_EQ(dr_metalib_source_from_user, dr_metalib_source_from(source));
    EXPECT_EQ(dr_metalib_source_state_not_analize, dr_metalib_source_state(source));

    EXPECT_STREQ("a", dr_metalib_source_name(source));
    EXPECT_STREQ("b/a.xml", dr_metalib_source_file(source));
    EXPECT_TRUE(NULL == dr_metalib_source_buf(source));
    EXPECT_EQ((size_t)0, dr_metalib_source_buf_capacity(source));
}

TEST_F(BuilderBasicTest, add_file_with_name) {
    dr_metalib_source_t source = dr_metalib_builder_add_file(m_builder, "c", "b/a.xml");
    ASSERT_TRUE(source);

    EXPECT_STREQ("c", dr_metalib_source_name(source));
}

TEST_F(BuilderBasicTest, add_buf_basic) {
    dr_metalib_source_t source = dr_metalib_builder_add_buf(m_builder, "a", dr_metalib_source_format_xml, "abc");
    ASSERT_TRUE(source);

    EXPECT_EQ(dr_metalib_source_type_memory, dr_metalib_source_type(source));
    EXPECT_EQ(dr_metalib_source_format_xml, dr_metalib_source_format(source));
    EXPECT_EQ(dr_metalib_source_from_user, dr_metalib_source_from(source));
    EXPECT_EQ(dr_metalib_source_state_not_analize, dr_metalib_source_state(source));

    EXPECT_STREQ("a", dr_metalib_source_name(source));
    EXPECT_TRUE(NULL == dr_metalib_source_file(source));
    EXPECT_STREQ("abc", (const char *)dr_metalib_source_buf(source));
    EXPECT_EQ((size_t)4, dr_metalib_source_buf_capacity(source));
}

TEST_F(BuilderBasicTest, add_file_prefix_unknown) {
    dr_metalib_source_t source = dr_metalib_builder_add_file(m_builder, NULL, "b/a.aa");
    ASSERT_TRUE(source == NULL);
}

TEST_F(BuilderBasicTest, source_find_basic) {
    dr_metalib_source_t source = dr_metalib_builder_add_file(m_builder, NULL, "b/a.xml");
    ASSERT_TRUE(source == dr_metalib_source_find(m_builder, "a"));
}

TEST_F(BuilderBasicTest, source_find_not_exist) {
    ASSERT_TRUE(NULL == dr_metalib_source_find(m_builder, "a"));
}

TEST_F(BuilderBasicTest, it_empty) {
    struct dr_metalib_source_it it;
    dr_metalib_builder_sources(&it, m_builder);

    EXPECT_TRUE(NULL == dr_metalib_source_next(&it));
}

TEST_F(BuilderBasicTest, it_basic) {
    dr_metalib_source_t source_a = dr_metalib_builder_add_file(m_builder, NULL, "a.xml");
    dr_metalib_source_t source_b = dr_metalib_builder_add_file(m_builder, NULL, "b.xml");

    struct dr_metalib_source_it it;
    dr_metalib_builder_sources(&it, m_builder);

    EXPECT_TRUE(source_a == dr_metalib_source_next(&it));
    EXPECT_TRUE(source_b == dr_metalib_source_next(&it));
    EXPECT_TRUE(NULL == dr_metalib_source_next(&it));
}
