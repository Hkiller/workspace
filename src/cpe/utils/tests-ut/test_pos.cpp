#include "BufferTest.hpp"

TEST_F(BufferTest, pos_begin_with_empty_trunk) {
    append_string("");
    append_string("");

    struct mem_buffer_pos l;
    mem_buffer_begin(&l, &m_buffer);

    struct mem_buffer_pos e;
    mem_buffer_end(&e, &m_buffer);

    EXPECT_TRUE(mem_pos_eq(&l, &e));
}

TEST_F(BufferTest, pos_valid_head) {
    append_string("a");

    struct mem_buffer_pos l;
    mem_buffer_begin(&l, &m_buffer);

    EXPECT_EQ(1, mem_pos_valide(&l));
}

TEST_F(BufferTest, pos_valid_end) {
    append_string("a");

    struct mem_buffer_pos l;
    mem_buffer_end(&l, &m_buffer);

    EXPECT_EQ(0, mem_pos_valide(&l));
}

TEST_F(BufferTest, pos_begin_end_ne) {
    append_string("a");

    struct mem_buffer_pos l;
    mem_buffer_begin(&l, &m_buffer);

    struct mem_buffer_pos r;
    mem_buffer_end(&r, &m_buffer);
    
    EXPECT_EQ(0, mem_pos_eq(&l, &r));
}

TEST_F(BufferTest, pos_begin_end_eq) {
    struct mem_buffer_pos l;
    mem_buffer_begin(&l, &m_buffer);

    struct mem_buffer_pos r;
    mem_buffer_end(&r, &m_buffer);

    EXPECT_EQ(1, mem_pos_eq(&l, &r));
}

TEST_F(BufferTest, pos_at) {
    append_string("abc");

    struct mem_buffer_pos p;
    mem_pos_at(&p, &m_buffer, 0);
    EXPECT_EQ('a', mem_pos_data(&p));

    mem_pos_at(&p, &m_buffer, 1);
    EXPECT_EQ('b', mem_pos_data(&p));

    mem_pos_at(&p, &m_buffer, 2);
    EXPECT_EQ('c', mem_pos_data(&p));

    mem_pos_at(&p, &m_buffer, 3);
    EXPECT_FALSE(mem_pos_valide(&p));
}

TEST_F(BufferTest, pos_seek_fwd_basic) {
    struct mem_buffer_trunk * t1 = append_trunk("abc");

    struct mem_buffer_pos p;
    mem_buffer_begin(&p, &m_buffer);
    EXPECT_EQ(1, mem_pos_seek(&p, 1));

    EXPECT_TRUE(t1 == p.m_trunk);
    EXPECT_EQ((size_t)1, p.m_pos_in_trunk);
}

TEST_F(BufferTest, pos_seek_fwd_to_last) {
    struct mem_buffer_trunk * t1 = append_trunk("abc");

    struct mem_buffer_pos p;
    mem_buffer_begin(&p, &m_buffer);
    EXPECT_EQ(2, mem_pos_seek(&p, 2));

    EXPECT_TRUE(t1 == p.m_trunk);
    EXPECT_EQ((size_t)2, p.m_pos_in_trunk);
}

TEST_F(BufferTest, pos_seek_fwd_from_last) {
    append_trunk("abc");

    struct mem_buffer_pos p;
    mem_buffer_end(&p, &m_buffer);
    EXPECT_EQ(0, mem_pos_seek(&p, 2));

    EXPECT_TRUE(NULL == p.m_trunk);
    EXPECT_EQ((size_t)0, p.m_pos_in_trunk);
}

TEST_F(BufferTest, pos_seek_fwd_multi_trunk) {
    struct mem_buffer_trunk * t1 = append_trunk("a");
    struct mem_buffer_trunk * t2 = append_trunk("b");
    struct mem_buffer_trunk * t3 = append_trunk("c");

    struct mem_buffer_pos p;
    mem_buffer_begin(&p, &m_buffer);
    EXPECT_TRUE(t1 == p.m_trunk);
    EXPECT_EQ((size_t)0, p.m_pos_in_trunk);
    EXPECT_EQ('a', mem_pos_data(&p));

    EXPECT_EQ(1, mem_pos_seek(&p, 1));
    EXPECT_TRUE(t2 == p.m_trunk);
    EXPECT_EQ((size_t)0, p.m_pos_in_trunk);
    EXPECT_EQ('b', mem_pos_data(&p));

    EXPECT_EQ(1, mem_pos_seek(&p, 1));
    EXPECT_TRUE(t3 == p.m_trunk);
    EXPECT_EQ((size_t)0, p.m_pos_in_trunk);
    EXPECT_EQ('c', mem_pos_data(&p));

    EXPECT_EQ(1, mem_pos_seek(&p, 1));
    EXPECT_TRUE(NULL == p.m_trunk);
    EXPECT_EQ((size_t)0, p.m_pos_in_trunk);
    EXPECT_FALSE(mem_pos_valide(&p));
}

TEST_F(BufferTest, pos_seek_fwd_multi_trunk_with_empty_trunk_middle) {
    struct mem_buffer_trunk * t1 = append_trunk("a");
    append_trunk("");
    struct mem_buffer_trunk * t3 = append_trunk("c");

    struct mem_buffer_pos p;
    mem_buffer_begin(&p, &m_buffer);
    EXPECT_TRUE(t1 == p.m_trunk);
    EXPECT_EQ((size_t)0, p.m_pos_in_trunk);
    EXPECT_EQ('a', mem_pos_data(&p));

    EXPECT_EQ(1, mem_pos_seek(&p, 1));
    EXPECT_TRUE(t3 == p.m_trunk);
    EXPECT_EQ((size_t)0, p.m_pos_in_trunk);
    EXPECT_EQ('c', mem_pos_data(&p));

    EXPECT_EQ(1, mem_pos_seek(&p, 1));
    EXPECT_TRUE(NULL == p.m_trunk);
    EXPECT_EQ((size_t)0, p.m_pos_in_trunk);
    EXPECT_FALSE(mem_pos_valide(&p));
}

TEST_F(BufferTest, pos_seek_fwd_multi_trunk_with_empty_trunk_end) {
    struct mem_buffer_trunk * t1 = append_trunk("a");
    append_trunk("");
    append_trunk("");

    struct mem_buffer_pos p;
    mem_buffer_begin(&p, &m_buffer);
    EXPECT_TRUE(t1 == p.m_trunk);
    EXPECT_EQ((size_t)0, p.m_pos_in_trunk);
    EXPECT_EQ('a', mem_pos_data(&p));

    EXPECT_EQ(1, mem_pos_seek(&p, 1));
    EXPECT_TRUE(NULL == p.m_trunk);
    EXPECT_EQ((size_t)0, p.m_pos_in_trunk);
    EXPECT_FALSE(mem_pos_valide(&p));
}

TEST_F(BufferTest, pos_seek_fwd_empty) {
    struct mem_buffer_pos p;
    mem_buffer_begin(&p, &m_buffer);
    EXPECT_FALSE(mem_pos_valide(&p));

    EXPECT_EQ(0, mem_pos_seek(&p, 1));
    EXPECT_TRUE(NULL == p.m_trunk);
    EXPECT_EQ((size_t)0, p.m_pos_in_trunk);
    EXPECT_FALSE(mem_pos_valide(&p));
}

TEST_F(BufferTest, pos_seek_back_basic) {
    struct mem_buffer_trunk * t1 = append_trunk("abc");

    struct mem_buffer_pos p;
    mem_buffer_end(&p, &m_buffer);
    EXPECT_FALSE(mem_pos_valide(&p));

    EXPECT_EQ(-1, mem_pos_seek(&p, -1));
    EXPECT_EQ('c', mem_pos_data(&p));

    EXPECT_TRUE(t1 == p.m_trunk);
    EXPECT_EQ((size_t)2, p.m_pos_in_trunk);
}

TEST_F(BufferTest, pos_seek_back_to_head) {
    struct mem_buffer_trunk * t1 = append_trunk("abc");

    struct mem_buffer_pos p;
    mem_buffer_end(&p, &m_buffer);
    EXPECT_EQ(-3, mem_pos_seek(&p, -3));

    EXPECT_TRUE(t1 == p.m_trunk);
    EXPECT_EQ((size_t)0, p.m_pos_in_trunk);
    EXPECT_EQ('a', mem_pos_data(&p));
}

TEST_F(BufferTest, pos_seek_back_from_head) {
    struct mem_buffer_trunk * t1 = append_trunk("abc");

    struct mem_buffer_pos p;
    mem_buffer_begin(&p, &m_buffer);
    EXPECT_EQ(0, mem_pos_seek(&p, -2));

    EXPECT_TRUE(t1 == p.m_trunk);
    EXPECT_EQ((size_t)0, p.m_pos_in_trunk);
}

TEST_F(BufferTest, pos_seek_back_multi_trunk) {
    append_trunk("a");
    append_trunk("b");
    append_trunk("c");

    struct mem_buffer_pos p;
    mem_buffer_end(&p, &m_buffer);

    EXPECT_EQ(-1, mem_pos_seek(&p, -1));
    EXPECT_EQ('c', mem_pos_data(&p));

    EXPECT_EQ(-1, mem_pos_seek(&p, -1));
    EXPECT_EQ('b', mem_pos_data(&p));

    EXPECT_EQ(-1, mem_pos_seek(&p, -1));
    EXPECT_EQ('a', mem_pos_data(&p));
}

TEST_F(BufferTest, pos_seek_back_multi_trunk_with_empty_trunk_middle) {
    append_trunk("a");
    append_trunk("");
    append_trunk("c");

    struct mem_buffer_pos p;
    mem_buffer_end(&p, &m_buffer);

    EXPECT_EQ(-1, mem_pos_seek(&p, -1));
    EXPECT_EQ('c', mem_pos_data(&p));

    EXPECT_EQ(-1, mem_pos_seek(&p, -1));
    EXPECT_EQ('a', mem_pos_data(&p));
}

TEST_F(BufferTest, pos_seek_back_multi_trunk_with_empty_trunk_end) {
    append_trunk("a");
    append_trunk("");
    append_trunk("");

    struct mem_buffer_pos p;
    mem_buffer_end(&p, &m_buffer);

    EXPECT_EQ(-1, mem_pos_seek(&p, -1));
    EXPECT_EQ('a', mem_pos_data(&p));
}

TEST_F(BufferTest, pos_seek_back_multi_trunk_all_empty_trunk) {
    append_trunk("");
    append_trunk("");
    append_trunk("");

    struct mem_buffer_pos p;
    mem_buffer_end(&p, &m_buffer);

    EXPECT_EQ(0, mem_pos_seek(&p, -1));

    struct mem_buffer_pos e;
    mem_buffer_end(&e, &m_buffer);
    EXPECT_TRUE(mem_pos_eq(&p, &e));
}

TEST_F(BufferTest, pos_seek_back_empty) {
    struct mem_buffer_pos p;
    mem_buffer_end(&p, &m_buffer);

    EXPECT_EQ(0, mem_pos_seek(&p, -1));

    struct mem_buffer_pos e;
    mem_buffer_end(&e, &m_buffer);
    EXPECT_TRUE(mem_pos_eq(&p, &e));
}

TEST_F(BufferTest, pos_diff_basic) {
    append_trunk("abc");

    struct mem_buffer_pos b;
    mem_buffer_begin(&b, &m_buffer);

    struct mem_buffer_pos e;
    mem_buffer_end(&e, &m_buffer);

    EXPECT_EQ(3, mem_pos_diff(&b, &e));
    EXPECT_EQ(-3, mem_pos_diff(&e, &b));
}

TEST_F(BufferTest, pos_diff_empty) {
    struct mem_buffer_pos b;
    mem_buffer_begin(&b, &m_buffer);

    struct mem_buffer_pos e;
    mem_buffer_begin(&e, &m_buffer);

    EXPECT_EQ(0, mem_pos_diff(&b, &e));
    EXPECT_EQ(0, mem_pos_diff(&e, &b));
}

TEST_F(BufferTest, pos_diff_same_trunk) {
    append_trunk("abc");

    struct mem_buffer_pos b;
    mem_pos_at(&b, &m_buffer, 2);

    struct mem_buffer_pos e;
    mem_pos_at(&e, &m_buffer, 1);

    EXPECT_EQ(-1, mem_pos_diff(&b, &e));
    EXPECT_EQ(1, mem_pos_diff(&e, &b));
}

TEST_F(BufferTest, pos_diff_same_multi_trunk) {
    append_trunk("a");
    append_trunk("b");
    append_trunk("c");

    struct mem_buffer_pos b;
    mem_buffer_begin(&b, &m_buffer);

    struct mem_buffer_pos e;
    mem_buffer_end(&e, &m_buffer);

    EXPECT_EQ(3, mem_pos_diff(&b, &e));
    EXPECT_EQ(-3, mem_pos_diff(&e, &b));
}

TEST_F(BufferTest, pos_diff_multi_trunk_from_middle) {
    append_trunk("a");
    append_trunk("b");
    append_trunk("c");

    struct mem_buffer_pos b;
    mem_pos_at(&b, &m_buffer, 1);

    struct mem_buffer_pos e;
    mem_buffer_end(&e, &m_buffer);

    EXPECT_EQ(2, mem_pos_diff(&b, &e));
    EXPECT_EQ(-2, mem_pos_diff(&e, &b));
}

TEST_F(BufferTest, pos_diff_multi_trunk_to_middle) {
    append_trunk("a");
    append_trunk("b");
    append_trunk("c");

    struct mem_buffer_pos b;
    mem_buffer_begin(&b, &m_buffer);

    struct mem_buffer_pos e;
    mem_pos_at(&e, &m_buffer, 1);

    EXPECT_EQ(1, mem_pos_diff(&b, &e));
    EXPECT_EQ(-1, mem_pos_diff(&e, &b));
}

TEST_F(BufferTest, pos_diff_multi_trunk_middle_to_middle) {
    append_trunk("a");
    append_trunk("b");
    append_trunk("c");

    struct mem_buffer_pos b;
    mem_pos_at(&b, &m_buffer, 1);

    struct mem_buffer_pos e;
    mem_pos_at(&e, &m_buffer, 2);

    EXPECT_EQ(1, mem_pos_diff(&b, &e));
    EXPECT_EQ(-1, mem_pos_diff(&e, &b));
}

TEST_F(BufferTest, pos_insert_in_empty) {
    struct mem_buffer_pos p;
    mem_pos_at(&p, &m_buffer, 3);

    EXPECT_EQ(3, mem_pos_insert(&p, "abc", 3));
    EXPECT_FALSE(mem_pos_valide(&p));

    append_zero();
    EXPECT_STREQ("abc", as_string());
}

TEST_F(BufferTest, pos_insert_at_trunk_first) {
    append_trunk("def");

    struct mem_buffer_pos p;
    mem_pos_at(&p, &m_buffer, 0);

    EXPECT_EQ(3, mem_pos_insert(&p, "abc", 3));
    EXPECT_EQ((size_t)6, buffer_size());
    EXPECT_EQ('d', mem_pos_data(&p));

    append_zero();
    EXPECT_STREQ("abcdef", as_string());
}

TEST_F(BufferTest, pos_insert_at_trunk_middle) {
    append_trunk("def");

    struct mem_buffer_pos p;
    mem_pos_at(&p, &m_buffer, 1);

    EXPECT_EQ(3, mem_pos_insert(&p, "abc", 3));
    EXPECT_EQ('e', mem_pos_data(&p));

    append_zero();
    EXPECT_STREQ("dabcef", as_string());
}

TEST_F(BufferTest, pos_insert_at_trunk_last) {
    append_trunk("def");

    struct mem_buffer_pos p;
    mem_pos_at(&p, &m_buffer, 3);

    EXPECT_EQ(3, mem_pos_insert(&p, "abc", 3));
    EXPECT_FALSE(mem_pos_valide(&p));

    append_zero();
    EXPECT_STREQ("defabc", as_string());
}

TEST_F(BufferTest, pos_insert_multi_trunk_at_trunk_first) {
    append_trunk("a");
    append_trunk("bc");
    append_trunk("d");

    struct mem_buffer_pos p;
    mem_pos_at(&p, &m_buffer, 1);

    EXPECT_EQ(3, mem_pos_insert(&p, "efg", 3));
    EXPECT_EQ('b', mem_pos_data(&p));

    append_zero();
    EXPECT_STREQ("aefgbcd", as_string());
}

TEST_F(BufferTest, pos_insert_multi_trunk_at_trunk_last) {
    append_trunk("a");
    append_trunk("bc");
    append_trunk("d");

    struct mem_buffer_pos p;
    mem_pos_at(&p, &m_buffer, 3);

    EXPECT_EQ(3, mem_pos_insert(&p, "efg", 3));
    EXPECT_EQ('d', mem_pos_data(&p));

    append_zero();
    EXPECT_STREQ("abcefgd", as_string());
}

TEST_F(BufferTest, pos_insert_not_use_empty_trunk) {
    append_trunk("a");
    append_trunk("bc");
    append_trunk(NULL);
    append_trunk("d");

    struct mem_buffer_pos p;
    mem_pos_at(&p, &m_buffer, 3);

    EXPECT_EQ(3, mem_pos_insert(&p, "efg", 3));
    EXPECT_EQ((size_t)5, trunk_count());
    EXPECT_EQ('d', mem_pos_data(&p));

    append_zero();
    EXPECT_STREQ("abcefgd", as_string());
}

TEST_F(BufferTest, pos_insert_enouth_in_current_middle) {
    append_trunk("a");
    append_trunk("bc");
    append_trunk("d");

    struct mem_buffer_pos p;
    mem_pos_at(&p, &m_buffer, 2);

    EXPECT_EQ(1, mem_pos_insert(&p, "efg", 1));
    EXPECT_EQ('c', mem_pos_data(&p));

    append_zero();
    EXPECT_STREQ("abecd", as_string());
}


TEST_F(BufferTest, pos_insert_zero) {
    append_trunk("def");

    struct mem_buffer_pos p;
    mem_pos_at(&p, &m_buffer, 0);

    EXPECT_EQ(0, mem_pos_insert(&p, "", 0));
    EXPECT_EQ('d', mem_pos_data(&p));

    append_zero();
    EXPECT_STREQ("def", as_string());
}

