#include <assert.h>
#include "rank_f_svr_ops.h"

rank_f_svr_index_buf_t rank_f_svr_index_buf_alloc(rank_f_svr_t svr) {
    rank_f_svr_index_buf_t r;

    if (svr->m_free_bufs == NULL) {
        rank_f_svr_index_buf_t buf;
        uint32_t buf_size = svr->m_page_size;

        buf = mem_alloc(svr->m_alloc, buf_size);
        if (buf == NULL) {
            CPE_ERROR(svr->m_em, "%s: index buf alloc, alloc buf fail!", rank_f_svr_name(svr));
            return NULL;
        }

        bzero(buf, buf_size);

        svr->m_buf_page_count ++;

        /*将第一块链接到buf头的列表中，不使用 */
        svr->m_buf_count++;
        buf->m_next = svr->m_buf_heads;
        svr->m_buf_heads = buf;

        /*将剩余块放入空闲列表 */
        for(buf++, buf_size -= sizeof(struct rank_f_svr_index_buf);
            buf_size >= sizeof(struct rank_f_svr_index_buf);
            buf++, buf_size -= sizeof(struct rank_f_svr_index_buf))
        {
            svr->m_buf_count++;
            svr->m_buf_free_count++;

            buf->m_next = svr->m_free_bufs;
            svr->m_free_bufs = buf;
        }
    }

    assert(svr->m_free_bufs);

    svr->m_buf_using_count++;
    svr->m_buf_free_count--;

    r = svr->m_free_bufs;
    svr->m_free_bufs = r->m_next;

    r->m_next = NULL;
    r->m_record_count = 0;

    return r;
}

void rank_f_svr_index_buf_free(rank_f_svr_t svr, rank_f_svr_index_buf_t buf) {
    svr->m_buf_using_count--;
    svr->m_buf_free_count++;

    assert(buf->m_next == NULL);
    assert(buf->m_record_count == 0);

    buf->m_next = svr->m_free_bufs;
    svr->m_free_bufs = buf;
}

void rank_f_svr_index_buf_release_all(rank_f_svr_t svr) {
    rank_f_svr_index_buf_t buf_head = svr->m_buf_heads;

    while(buf_head) {
        rank_f_svr_index_buf_t buf_next = buf_head->m_next;
        mem_free(svr->m_alloc, buf_head);
        buf_head = buf_next;
    }

    svr->m_buf_heads = NULL;
    svr->m_free_bufs = NULL;
}


