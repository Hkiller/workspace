#include "DebugAllocratorTest.hpp"

TEST_F(DebugAllocratorTest, basic) {
    EXPECT_EQ(0, mem_allocrator_debug_alloc_count(m_allocrator));
    EXPECT_EQ(0, mem_allocrator_debug_alloc_size(m_allocrator));
    EXPECT_EQ(0, mem_allocrator_debug_free_count(m_allocrator));
    EXPECT_EQ(0, mem_allocrator_debug_free_size(m_allocrator));
}

TEST_F(DebugAllocratorTest, alloc_basic) {
    void * r = mem_alloc(m_allocrator, 123);
    
    EXPECT_TRUE(r);
    EXPECT_EQ(1, mem_allocrator_debug_alloc_count(m_allocrator));
    EXPECT_EQ(123, mem_allocrator_debug_alloc_size(m_allocrator));
    EXPECT_EQ(0, mem_allocrator_debug_free_count(m_allocrator));
    EXPECT_EQ(0, mem_allocrator_debug_free_size(m_allocrator));
}

TEST_F(DebugAllocratorTest, free_basic) {
    mem_free(m_allocrator, mem_alloc(m_allocrator, 123));
    
    EXPECT_EQ(1, mem_allocrator_debug_alloc_count(m_allocrator));
    EXPECT_EQ(123, mem_allocrator_debug_alloc_size(m_allocrator));
    EXPECT_EQ(1, mem_allocrator_debug_free_count(m_allocrator));
    EXPECT_EQ(123, mem_allocrator_debug_free_size(m_allocrator));
}

TEST_F(DebugAllocratorTest, dump_basic) {
    mem_alloc(m_allocrator, 123);

    EXPECT_TRUE(NULL != dump());
}

TEST_F(DebugAllocratorTest, dump_empty) {
    EXPECT_STREQ(
        "",
        dump());
}

