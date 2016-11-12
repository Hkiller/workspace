#include <assert.h>
#include "cpe/aom/aom_obj_mgr.h"
#include "rank_f_svr_ops.h"

rank_f_svr_index_t rank_f_svr_index_alloc(rank_f_svr_t svr, uint64_t user_id, uint8_t index_id) {
    rank_f_svr_index_t r;

    if (TAILQ_EMPTY(&svr->m_free_indexes)) {
        rank_f_svr_index_t index;
        uint32_t index_size = svr->m_page_size;

        index = mem_alloc(svr->m_alloc, index_size);
        if (index == NULL) {
            CPE_ERROR(svr->m_em, "%s: index alloc, alloc index fail!", rank_f_svr_name(svr));
            return NULL;
        }

        bzero(index, index_size);

        svr->m_index_page_count ++;

        /*将第一块链接到index头的列表中，不使用 */
        svr->m_index_count++;
        TAILQ_INSERT_TAIL(&svr->m_index_heads, index, m_next);

        /*将剩余块放入空闲列表 */
        for(index++, index_size -= sizeof(struct rank_f_svr_index);
            index_size >= sizeof(struct rank_f_svr_index);
            index++, index_size -= sizeof(struct rank_f_svr_index))
        {
            svr->m_index_count++;
            svr->m_index_free_count++;

            TAILQ_INSERT_TAIL(&svr->m_free_indexes, index, m_next);
        }
    }

    r = TAILQ_FIRST(&svr->m_free_indexes);
    assert(r);
    TAILQ_REMOVE(&svr->m_free_indexes, r, m_next);

    r->m_user_id = user_id;
    r->m_index_id = index_id;
    r->m_record_count = 0;
    r->m_bufs = NULL;
    r->m_last_op_time = rank_f_svr_cur_time(svr);

    cpe_hash_entry_init(&r->m_hh);
    if (cpe_hash_table_insert_unique(&svr->m_indexes, r) != 0) {
        CPE_ERROR(svr->m_em, "%s: index alloc, alloc index fail!", rank_f_svr_name(svr));
        TAILQ_INSERT_HEAD(&svr->m_free_indexes, r, m_next);
        return NULL;
    }

    if (index_id != 0) {
        TAILQ_INSERT_TAIL(&svr->m_indexes_for_check, r, m_next);
    }

    svr->m_index_free_count--;
    svr->m_index_using_count++;
    return r;
}

void rank_f_svr_index_clear_records(rank_f_svr_t svr, rank_f_svr_index_t index) {
    while(index->m_bufs) {
        rank_f_svr_index_buf_t buf;

        buf = index->m_bufs;
        index->m_bufs = buf->m_next;
        buf->m_next = NULL;
        buf->m_record_count = 0;

        rank_f_svr_index_buf_free(svr, buf);
    }

    index->m_record_count = 0;
    index->m_bufs = NULL;
}

void rank_f_svr_index_destory_records(rank_f_svr_t svr, rank_f_svr_index_t index) {
    while(index->m_bufs) {
        rank_f_svr_index_buf_t buf;
        uint32_t i;

        buf = index->m_bufs;
        index->m_bufs = buf->m_next;

        for(i = 0; i < buf->m_record_count; ++i) {
            aom_obj_free(svr->m_record_mgr, buf->m_records[i]);
        }

        buf->m_next = NULL;
        buf->m_record_count = 0;

        rank_f_svr_index_buf_free(svr, buf);
    }

    index->m_record_count = 0;
    index->m_bufs = NULL;
}

void rank_f_svr_index_free(rank_f_svr_t svr, rank_f_svr_index_t index) {
    if (index->m_index_id != 0) {
        TAILQ_REMOVE(&svr->m_indexes_for_check, index, m_next);
    }

    assert(index->m_bufs == NULL);

    cpe_hash_table_remove_by_ins(&svr->m_indexes, index);

    svr->m_index_free_count++;
    svr->m_index_using_count--;

    TAILQ_INSERT_HEAD(&svr->m_free_indexes, index, m_next);
}

void rank_f_svr_index_free_all(rank_f_svr_t svr) {
    struct cpe_hash_it index_it;
    rank_f_svr_index_t index;

    cpe_hash_it_init(&index_it, &svr->m_indexes);

    index = cpe_hash_it_next(&index_it);
    while(index) {
        rank_f_svr_index_t free_index = index;
        index = cpe_hash_it_next(&index_it);

        if (free_index->m_index_id == 0) {
            rank_f_svr_index_destory_records(svr, free_index);
        }
        else {
            rank_f_svr_index_clear_records(svr, free_index);
        }

        rank_f_svr_index_free(svr, free_index);
    }

    while(!TAILQ_EMPTY(&svr->m_index_heads)) {
        rank_f_svr_index_t head = TAILQ_FIRST(&svr->m_index_heads);
        TAILQ_REMOVE(&svr->m_index_heads, head, m_next);

        mem_free(svr->m_alloc, head);
    }

    TAILQ_INIT(&svr->m_free_indexes);
}

rank_f_svr_index_t rank_f_svr_index_find(rank_f_svr_t svr, uint64_t user_id, uint8_t index_id) {
    struct rank_f_svr_index key;
    key.m_user_id = user_id;
    key.m_index_id = index_id;

    return cpe_hash_table_find(&svr->m_indexes, &key);
}

uint32_t rank_f_svr_index_hash(rank_f_svr_index_t index) {
    return ((uint32_t)index->m_user_id) << 8 | ((uint32_t)index->m_index_id);
}

int rank_f_svr_index_eq(rank_f_svr_index_t l, rank_f_svr_index_t r) {
    return l->m_user_id == r->m_user_id && l->m_index_id == r->m_index_id;
}
