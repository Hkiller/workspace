#include "BufMgrTest.hpp"

extern struct pom_backend g_pom_backend_mem;

TEST_F(BufMgrTest, init_page_size_too_small) {
    EXPECT_EQ(-1, init(1, 40));
}

TEST_F(BufMgrTest, init_buf_size_too_small) {
    EXPECT_EQ(-1, init(41, 40));
}

TEST_F(BufMgrTest, get_page_empty_no_source) {
    ASSERT_EQ(0, init(12, 40));
    EXPECT_TRUE(NULL == page_get());

    EXPECT_TRUE(t_em_have_errno(pom_no_buffer));
}

TEST_F(BufMgrTest, get_page_auto_create) {
    ASSERT_EQ(0, init(12, 40));
    EXPECT_EQ(0, pom_buffer_mgr_set_backend(&m_bufMgr, &g_pom_backend_mem, t_allocrator()));

    EXPECT_TRUE(page_get());
}

TEST_F(BufMgrTest, set_source_no_buf) {
    ASSERT_EQ(0, init(12, 40));
    EXPECT_EQ(0, pom_buffer_mgr_set_backend(&m_bufMgr, &g_pom_backend_mem, t_allocrator()));
    EXPECT_EQ(0, pom_buffer_mgr_set_backend(&m_bufMgr, NULL, NULL));
}

TEST_F(BufMgrTest, get_page_buf_left) {
    ASSERT_EQ(0, init(12, 48));

    char * p = (char *)t_tmp_alloc(49);

    EXPECT_EQ(
        0
        , pom_buffer_mgr_add_new_buffer(&m_bufMgr, (pom_buffer_id_t)p, t_em()));

    EXPECT_TRUE(p == page_get());
    EXPECT_TRUE(p + 12 == page_get());
    EXPECT_TRUE(p + 24 == page_get());
    EXPECT_TRUE(p + 36 == page_get());
    EXPECT_TRUE(NULL == page_get());
}

TEST_F(BufMgrTest, set_source_have_buf) {
    init(12, 40);
    EXPECT_EQ(0, pom_buffer_mgr_set_backend(&m_bufMgr, &g_pom_backend_mem, t_allocrator()));

    EXPECT_TRUE(page_get());

    EXPECT_EQ(-1, pom_buffer_mgr_set_backend(&m_bufMgr, NULL, NULL));
}

TEST_F(BufMgrTest, add_page_no_source) {
    init(12, 48);

    char * p = (char *)t_tmp_alloc(48);

    EXPECT_EQ(
        0
        , pom_buffer_mgr_add_new_buffer(&m_bufMgr, (pom_buffer_id_t)p, t_em()));

    EXPECT_TRUE(p == page_get());
    EXPECT_TRUE(p + 12 == page_get());
    EXPECT_TRUE(p + 24 == page_get());
    EXPECT_TRUE(p + 36 == page_get());
    EXPECT_TRUE(NULL == page_get());
}

TEST_F(BufMgrTest, find_page_empty) {
    init(12, 40);

    EXPECT_TRUE(
        NULL ==
        pom_buffer_mgr_find_page(&m_bufMgr, (void*)23121));
}

TEST_F(BufMgrTest, find_page_buf_begin) {
    init(12, 48);

    char * p = (char *)t_tmp_alloc(48);

    EXPECT_EQ(
        0
        , pom_buffer_mgr_add_new_buffer(&m_bufMgr, (pom_buffer_id_t)p, t_em()));

    EXPECT_TRUE(
        p ==
        pom_buffer_mgr_find_page(&m_bufMgr, p));
}

TEST_F(BufMgrTest, find_page_buf_middle) {
    init(12, 48);

    char * p = (char *)t_tmp_alloc(48);

    EXPECT_EQ(
        0
        , pom_buffer_mgr_add_new_buffer(&m_bufMgr, (pom_buffer_id_t)p, t_em()));

    EXPECT_TRUE(
        (p + 24) ==
        pom_buffer_mgr_find_page(&m_bufMgr, p + 24));
}

TEST_F(BufMgrTest, find_page_buf_last_page) {
    init(12, 48);

    char * p = (char *)t_tmp_alloc(48);

    EXPECT_EQ(
        0
        , pom_buffer_mgr_add_new_buffer(&m_bufMgr, (pom_buffer_id_t)p, t_em()));

    EXPECT_TRUE(
        (p + 36) ==
        pom_buffer_mgr_find_page(&m_bufMgr, p + 36));
}

TEST_F(BufMgrTest, find_page_buf_after_buf) {
    init(12, 48);

    char * p = (char *)t_tmp_alloc(48);

    EXPECT_EQ(
        0
        , pom_buffer_mgr_add_new_buffer(&m_bufMgr, (pom_buffer_id_t)p, t_em()));

    EXPECT_TRUE(
        NULL ==
        pom_buffer_mgr_find_page(&m_bufMgr, p + 48));
}

TEST_F(BufMgrTest, find_page_buf_before_buf) {
    init(12, 48);

    char * p = (char *)t_tmp_alloc(48);

    EXPECT_EQ(
        0
        , pom_buffer_mgr_add_new_buffer(&m_bufMgr, (pom_buffer_id_t)p, t_em()));

    EXPECT_TRUE(
        NULL ==
        pom_buffer_mgr_find_page(&m_bufMgr, p - 1));
}

TEST_F(BufMgrTest, find_page_page_middle) {
    init(12, 48);

    char * p = (char *)t_tmp_alloc(48);

    EXPECT_EQ(
        0
        , pom_buffer_mgr_add_new_buffer(&m_bufMgr, (pom_buffer_id_t)p, t_em()));

    EXPECT_TRUE(
        p ==
        pom_buffer_mgr_find_page(&m_bufMgr, p + 1));
}

TEST_F(BufMgrTest, find_page_page_last) {
    init(12, 48);

    char * p = (char *)t_tmp_alloc(48);

    EXPECT_EQ(
        0
        , pom_buffer_mgr_add_new_buffer(&m_bufMgr, (pom_buffer_id_t)p, t_em()));

    EXPECT_TRUE(
        p ==
        pom_buffer_mgr_find_page(&m_bufMgr, p + 10 - 1));
}

TEST_F(BufMgrTest, find_page_multi_buf) {
    init(12, 48);

    char * p = (char *)t_tmp_alloc(96);

    EXPECT_EQ(
        0
        , pom_buffer_mgr_add_new_buffer(&m_bufMgr, (pom_buffer_id_t)p, t_em()));

    EXPECT_EQ(
        0
        , pom_buffer_mgr_add_new_buffer(&m_bufMgr, (pom_buffer_id_t)(p + 40), t_em()));

    EXPECT_TRUE(
        (p + 48) ==
        pom_buffer_mgr_find_page(&m_bufMgr, p + 48));

    EXPECT_TRUE(
        (p + 84) ==
        pom_buffer_mgr_find_page(&m_bufMgr, p + 84));
}

TEST_F(BufMgrTest, find_page_multi_buf_buf_page_left) {
    init(12, 50);

    char * p = (char *)t_tmp_alloc(100);

    EXPECT_EQ(
        0
        , pom_buffer_mgr_add_new_buffer(&m_bufMgr, (pom_buffer_id_t)p, t_em()));

    EXPECT_EQ(
        0
        , pom_buffer_mgr_add_new_buffer(&m_bufMgr, (pom_buffer_id_t)(p + 50), t_em()));

    EXPECT_TRUE(
        NULL ==
        pom_buffer_mgr_find_page(&m_bufMgr, p + 48));

    EXPECT_TRUE(
        NULL ==
        pom_buffer_mgr_find_page(&m_bufMgr, p + 49));

    EXPECT_TRUE(
        (p + 50) ==
        pom_buffer_mgr_find_page(&m_bufMgr, p + 50));
}
