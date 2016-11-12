#include "../buffer_private.h"
#include "BufferTest.hpp"

TEST_F(BufferTest, trunk_create) {
    struct mem_buffer_trunk * trunk = 
        mem_buffer_append_trunk(&m_buffer, 3);

    EXPECT_EQ((size_t)3, trunk->m_capacity);
    EXPECT_EQ((size_t)0, trunk->m_size);
    EXPECT_TRUE(trunk->m_allocrator == m_buffer.m_default_allocrator);
}

TEST_F(BufferTest, trunk_append_basic) {
    struct mem_buffer_trunk * trunk = 
        mem_buffer_append_trunk(&m_buffer, 3);

    const char * data = "abcde";

    EXPECT_EQ((size_t)3, mem_trunk_append(&m_buffer, trunk, data, 3));
    EXPECT_EQ((size_t)3, trunk->m_size);
    EXPECT_EQ((size_t)3, mem_buffer_size(&m_buffer));
}

TEST_F(BufferTest, trunk_append_twice) {
    struct mem_buffer_trunk * trunk = 
        mem_buffer_append_trunk(&m_buffer, 10);

    const char * data = "abcde";

    EXPECT_EQ((size_t)3, mem_trunk_append(&m_buffer, trunk, data, 3));
    EXPECT_EQ((size_t)3, mem_trunk_append(&m_buffer, trunk, data, 3));
    EXPECT_EQ((size_t)6, trunk->m_size);
    EXPECT_EQ((size_t)6, mem_buffer_size(&m_buffer));

    char buf = 0;
    EXPECT_EQ((size_t)1, mem_trunk_append(&m_buffer, trunk, &buf, 1));
    EXPECT_EQ((size_t)7, trunk->m_size);
    EXPECT_EQ((size_t)7, mem_buffer_size(&m_buffer));

    EXPECT_STREQ("abcabc", as_string());
}

TEST_F(BufferTest, trunk_append_overflow) {
    struct mem_buffer_trunk * trunk = 
        mem_buffer_append_trunk(&m_buffer, 2);

    const char * data = "abcde";

    EXPECT_EQ((size_t)2, mem_trunk_append(&m_buffer, trunk, data, 3));
    EXPECT_EQ((size_t)2, trunk->m_size);
    EXPECT_EQ((size_t)2, mem_buffer_size(&m_buffer));
}

TEST_F(BufferTest, trunk_append_twice_overflow) {
    struct mem_buffer_trunk * trunk = 
        mem_buffer_append_trunk(&m_buffer, 5);

    const char * data = "abcde";

    EXPECT_EQ((size_t)3, mem_trunk_append(&m_buffer, trunk, data, 3));
    EXPECT_EQ((size_t)3, trunk->m_size);
    EXPECT_EQ((size_t)3, mem_buffer_size(&m_buffer));


    EXPECT_EQ((size_t)2, mem_trunk_append(&m_buffer, trunk, data, 3));
    EXPECT_EQ((size_t)5, trunk->m_size);
    EXPECT_EQ((size_t)5, mem_buffer_size(&m_buffer));
}

TEST_F(BufferTest, trunk_free) {
    struct mem_buffer_trunk * trunk = 
        mem_buffer_append_trunk(&m_buffer, 5);

    const char * data = "abcde";

    EXPECT_EQ((size_t)3, mem_trunk_append(&m_buffer, trunk, data, 3));
    EXPECT_EQ((size_t)3, trunk->m_size);
    EXPECT_EQ((size_t)3, mem_buffer_size(&m_buffer));

    mem_trunk_free(&m_buffer, trunk);
    EXPECT_EQ((size_t)0, mem_buffer_size(&m_buffer));
}

TEST_F(BufferTest, trunk_free_middle) {
    const char * data = "abcde";

    struct mem_buffer_trunk * trunk1 = 
        mem_buffer_append_trunk(&m_buffer, 5);
    mem_trunk_append(&m_buffer, trunk1, data, 3);

    struct mem_buffer_trunk * trunk2 = 
        mem_buffer_append_trunk(&m_buffer, 5);
    mem_trunk_append(&m_buffer, trunk2, data, 4);

    EXPECT_EQ((size_t)7, mem_buffer_size(&m_buffer));

    mem_trunk_free(&m_buffer, trunk2);
    EXPECT_EQ((size_t)3, mem_buffer_size(&m_buffer));
}

TEST_F(BufferTest, trunk_count_basic) {
    append_trunk("");
    append_trunk("");
    append_trunk("");

    EXPECT_EQ((size_t)3, mem_buffer_trunk_count(&m_buffer));
}

TEST_F(BufferTest, trunk_count_empty) {
    EXPECT_EQ((size_t)0, mem_buffer_trunk_count(&m_buffer));
}

TEST_F(BufferTest, trunk_at_basic) {
    struct mem_buffer_trunk * t1 = append_trunk("");
    struct mem_buffer_trunk * t2 = append_trunk("");
    struct mem_buffer_trunk * t3 = append_trunk("");

    EXPECT_TRUE(t1 == mem_buffer_trunk_at(&m_buffer, 0));
    EXPECT_TRUE(t2 == mem_buffer_trunk_at(&m_buffer, 1));
    EXPECT_TRUE(t3 == mem_buffer_trunk_at(&m_buffer, 2));
    EXPECT_TRUE(NULL == mem_buffer_trunk_at(&m_buffer, 3));
}

TEST_F(BufferTest, trunk_at_empty) {
    EXPECT_TRUE(NULL == mem_buffer_trunk_at(&m_buffer, 0));
}
