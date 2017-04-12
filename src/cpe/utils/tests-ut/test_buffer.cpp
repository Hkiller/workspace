#include "cpe/utils/string_utils.h"
#include "BufferTest.hpp"

TEST_F(BufferTest, append_empty) {
    const char * data = "test_string";
    
    EXPECT_EQ(strlen(data), append_string(data));
    EXPECT_EQ((size_t)1, append_zero());

    EXPECT_EQ(strlen(data) + 1, mem_buffer_size(&m_buffer));

    EXPECT_STREQ(data, as_string());
}

TEST_F(BufferTest, append_multi_trunk) {
    EXPECT_TRUE(append_trunk("a"));
    EXPECT_TRUE(append_trunk("b"));
    EXPECT_TRUE(append_trunk("c"));

    append_zero();

    EXPECT_EQ((size_t)4, mem_buffer_size(&m_buffer));
    EXPECT_STREQ("abc", as_string());
}

TEST_F(BufferTest, read_from_empty) {
    char buf[5];

    EXPECT_EQ(0, mem_buffer_read(buf, 5, &m_buffer));
}

TEST_F(BufferTest, read_basic) {
    char buf[5];

    EXPECT_TRUE(append_trunk("a"));
    EXPECT_TRUE(append_trunk("b"));
    EXPECT_TRUE(append_trunk("c"));

    EXPECT_EQ(3, mem_buffer_read(buf, 5, &m_buffer));
    buf[3] = 0;

    EXPECT_STREQ("abc", buf);
}

TEST_F(BufferTest, make_exactly_no_trunk) {
    EXPECT_TRUE(mem_buffer_make_exactly(&m_buffer) == NULL);
}

TEST_F(BufferTest, make_exactly_single_trunk) {
    char buf[5] = { 0 };

    struct mem_buffer_trunk * told = append_trunk("a");
    void * newDataAddr = mem_buffer_make_exactly(&m_buffer);
    EXPECT_TRUE(told == trunk_at(0));

    memcpy(buf, newDataAddr, mem_buffer_size(&m_buffer));

    EXPECT_STREQ("a", buf);
}

TEST_F(BufferTest, make_exactly_multi_trunk) {
    char buf[5] = { 0 };

    EXPECT_TRUE(append_trunk("a"));
    EXPECT_TRUE(append_trunk("b"));
    EXPECT_TRUE(append_trunk("c"));

    memcpy(buf,mem_buffer_make_exactly(&m_buffer), mem_buffer_size(&m_buffer));

    EXPECT_STREQ("abc", buf);
}

TEST_F(BufferTest, make_continuous_no_trunk) {
    EXPECT_TRUE(mem_buffer_make_continuous(&m_buffer, 0) == NULL);
}

TEST_F(BufferTest, make_continuous_single_trunk) {
    char buf[5] = { 0 };

    struct mem_buffer_trunk * told = append_trunk("a");
    void * newDataAddr = mem_buffer_make_continuous(&m_buffer, 0);
    EXPECT_TRUE(told == trunk_at(0));

    memcpy(buf, newDataAddr, mem_buffer_size(&m_buffer));

    EXPECT_STREQ("a", buf);
}

TEST_F(BufferTest, make_continuous_single_trunk_with_capacity) {
    char buf[5] = { 0 };

    struct mem_buffer_trunk * told = append_trunk("a");
    void * newDataAddr = mem_buffer_make_continuous(&m_buffer, 1024);
    EXPECT_TRUE(told != trunk_at(0));
    EXPECT_EQ((size_t)1, mem_buffer_size(&m_buffer));

    memcpy(buf, newDataAddr, mem_buffer_size(&m_buffer));

    EXPECT_STREQ("a", buf);
}

TEST_F(BufferTest, make_continuous_multi_trunk) {
    char buf[5] = { 0 };

    EXPECT_TRUE(append_trunk("a"));
    EXPECT_TRUE(append_trunk("b"));
    EXPECT_TRUE(append_trunk("c"));

    memcpy(buf,mem_buffer_make_continuous(&m_buffer, 0), mem_buffer_size(&m_buffer));

    EXPECT_STREQ("abc", buf);
}

TEST_F(BufferTest, alloc_empty) {
    void * p = mem_buffer_alloc(&m_buffer, 12);
    EXPECT_TRUE(p);
    EXPECT_EQ((size_t)12, mem_buffer_size(&m_buffer));

    cpe_str_dup((char*)p, 12, "abc");
    EXPECT_STREQ("abc", as_string());
}

TEST_F(BufferTest, set_size_empty) {
     mem_buffer_alloc(&m_buffer, 12);

     EXPECT_EQ(0, mem_buffer_set_size(&m_buffer, 12));
     EXPECT_EQ((size_t)12, mem_buffer_size(&m_buffer));
}

TEST_F(BufferTest, set_size_bigger) {
     mem_buffer_alloc(&m_buffer, 12);

     EXPECT_EQ(0, mem_buffer_set_size(&m_buffer, 14));
     EXPECT_EQ((size_t)14, mem_buffer_size(&m_buffer));
}

TEST_F(BufferTest, set_size_small) {
     mem_buffer_alloc(&m_buffer, 12);

     EXPECT_EQ(0, mem_buffer_set_size(&m_buffer, 10));
     EXPECT_EQ((size_t)10, mem_buffer_size(&m_buffer));
}

TEST_F(BufferTest, set_size_small_multi_trunk) {
    EXPECT_TRUE(append_trunk("aaa"));
    EXPECT_TRUE(append_trunk("bbb"));
    EXPECT_EQ((size_t)6, mem_buffer_size(&m_buffer));

    EXPECT_EQ(0, mem_buffer_set_size(&m_buffer, 1));
    EXPECT_EQ((size_t)1, mem_buffer_size(&m_buffer));

    EXPECT_EQ((size_t)2, mem_buffer_trunk_count(&m_buffer));
}

TEST_F(BufferTest, alloc_empty_null) {
    m_buffer.m_default_allocrator = mem_allocrator_null();
    EXPECT_FALSE(mem_buffer_alloc(&m_buffer, 12));
    EXPECT_EQ((size_t)0, mem_buffer_size(&m_buffer));
}

TEST_F(BufferTest, strdup_basic) {
    EXPECT_STREQ("abc", mem_buffer_strdup(&m_buffer, "abc"));
    EXPECT_EQ((size_t)4, mem_buffer_size(&m_buffer));
}

TEST_F(BufferTest, strdup_len_basic) {
    EXPECT_STREQ("ab", mem_buffer_strdup_len(&m_buffer, "abc", 2));
    EXPECT_EQ((size_t)3, mem_buffer_size(&m_buffer));
}

TEST_F(BufferTest, strcat_to_empty) {
    EXPECT_EQ(0, mem_buffer_strcat(&m_buffer, "abc"));
    EXPECT_STREQ("abc", (char*)mem_buffer_make_continuous(&m_buffer, 0));
    EXPECT_EQ((size_t)4, mem_buffer_size(&m_buffer));
}

TEST_F(BufferTest, strcat_to_str) {
    EXPECT_EQ(0, mem_buffer_strcat(&m_buffer, "abc"));
    EXPECT_STREQ("abc", (char*)mem_buffer_make_continuous(&m_buffer, 0));

    EXPECT_EQ(0, mem_buffer_strcat(&m_buffer, "def"));
    EXPECT_STREQ("abcdef", (char*)mem_buffer_make_continuous(&m_buffer, 0));

    EXPECT_EQ((size_t)7, mem_buffer_size(&m_buffer));
}

TEST_F(BufferTest, strcat_to_buf) {
    EXPECT_EQ((size_t)3, append_string("abc"));
    EXPECT_EQ((size_t)3, mem_buffer_size(&m_buffer));

    EXPECT_EQ(-1, mem_buffer_strcat(&m_buffer, "def"));
    EXPECT_EQ((size_t)3, mem_buffer_size(&m_buffer));
}

TEST_F(BufferTest, strcat_to_str_trunk_full) {
    struct mem_buffer_trunk * trunk1 =
        mem_buffer_append_trunk(&m_buffer, 7);

    EXPECT_EQ((size_t)4, mem_trunk_append(&m_buffer, trunk1, "abc", 4));

    EXPECT_EQ(0, mem_buffer_strcat(&m_buffer, "def"));
    EXPECT_STREQ("abcdef", (char*)mem_buffer_make_continuous(&m_buffer, 0));

    EXPECT_EQ((size_t)7, mem_buffer_size(&m_buffer));
    EXPECT_EQ((size_t)1, mem_buffer_trunk_count(&m_buffer));
}

TEST_F(BufferTest, strcat_to_str_trunk_to_next) {
    struct mem_buffer_trunk * trunk1 =
        mem_buffer_append_trunk(&m_buffer, 6);

    EXPECT_EQ((size_t)4, mem_trunk_append(&m_buffer, trunk1, "abc", 4));

    EXPECT_EQ(0, mem_buffer_strcat(&m_buffer, "def"));

    EXPECT_EQ((size_t)2, mem_buffer_trunk_count(&m_buffer));

    EXPECT_STREQ("abcdef", (char*)mem_buffer_make_continuous(&m_buffer, 0));

    EXPECT_EQ((size_t)7, mem_buffer_size(&m_buffer));
}
