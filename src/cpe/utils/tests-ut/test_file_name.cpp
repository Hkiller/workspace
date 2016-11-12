#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/file.h"

class FileNameTest : public testenv::fixture<> {
public:
    virtual void SetUp() {
        Base::SetUp();
        mem_buffer_init(&m_buffer, t_allocrator());
    }

    virtual void TearDown() {
        mem_buffer_clear(&m_buffer);
        Base::TearDown();
    }

    struct mem_buffer m_buffer;
};

TEST_F(FileNameTest, suffix_null) {
    EXPECT_STREQ(NULL, file_name_suffix(NULL));
}

TEST_F(FileNameTest, suffix_empty) {
    EXPECT_STREQ("", file_name_suffix(""));
}

TEST_F(FileNameTest, suffix_end_with_dot) {
    EXPECT_STREQ("", file_name_suffix("a."));
}

TEST_F(FileNameTest, suffix_end_only_dot) {
    EXPECT_STREQ("", file_name_suffix("."));
}

TEST_F(FileNameTest, suffix_dot_in_dir) {
    EXPECT_STREQ("", file_name_suffix("a.b/c"));
}

TEST_F(FileNameTest, suffix_basic) {
    EXPECT_STREQ("d", file_name_suffix("a.b/c.d"));
}

TEST_F(FileNameTest, base_null) {
    EXPECT_STREQ(NULL, file_name_base(NULL, &m_buffer));
    EXPECT_EQ(0, t_alloc_count());
}

TEST_F(FileNameTest, base_empty) {
    EXPECT_STREQ("", file_name_base("", &m_buffer));
    EXPECT_EQ(0, t_alloc_count());
}

TEST_F(FileNameTest, base_end_with_dot) {
    EXPECT_STREQ("a", file_name_base("a.", &m_buffer));
    EXPECT_EQ(1, t_alloc_count());
}

TEST_F(FileNameTest, base_end_only_dot) {
    EXPECT_STREQ("", file_name_base(".", &m_buffer));
    EXPECT_EQ(0, t_alloc_count());
}

TEST_F(FileNameTest, base_dot_in_dir) {
    EXPECT_STREQ("c", file_name_base("a.b/c", &m_buffer));
    EXPECT_EQ(0, t_alloc_count());
}

TEST_F(FileNameTest, base_basic) {
    EXPECT_STREQ("c", file_name_base("a.b/c.d", &m_buffer));
    EXPECT_EQ(1, t_alloc_count());
}

TEST_F(FileNameTest, no_dir_basic) {
    EXPECT_STREQ("c.d", file_name_no_dir("a.b/c.d"));
}

TEST_F(FileNameTest, no_dir_no_dir) {
    EXPECT_STREQ("c.d", file_name_no_dir("c.d"));
}

