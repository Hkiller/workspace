#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "rank_f_svr_ops.h"

static int rank_f_svr_record_cmp(rank_f_svr_index_info_t index_info, void const * l, void const * r) {
    uint8_t i;
    for(i = 0; i < index_info->m_sorter_count; ++i) {
        struct rank_f_svr_index_sorter * sorter = index_info->m_sorters + i;
        int ret = sorter->m_sort_fun(
            ((char *)l) + sizeof(SVR_RANK_F_RECORD) + sorter->m_data_start_pos,
            (char *)r + sizeof(SVR_RANK_F_RECORD) + sorter->m_data_start_pos);
        if (ret) return ret;
    }

    return 0;
}

int rank_f_svr_record_update(rank_f_svr_t svr, rank_f_svr_index_t index, void const * record) {
    rank_f_svr_index_info_t index_info;
    rank_f_svr_index_buf_t * buf;

    rank_f_svr_index_buf_t insert_buf;
    uint16_t insert_pos;
    SVR_RANK_F_RECORD * place_record;
    
    index_info = &svr->m_index_infos[index->m_index_id];
    assert(index_info->m_svr);

    insert_pos = 0;
    insert_buf = NULL;

    for(buf = &index->m_bufs;
        insert_buf == NULL /*没有找到插入块*/
            && *buf;
        buf = &(*buf)->m_next)
    {
        rank_f_svr_index_buf_t cur_buf = *buf;
        void ** last_record;
        int r;

        assert(cur_buf->m_record_count > 0);

        last_record = cur_buf->m_records + cur_buf->m_record_count - 1;
        r = rank_f_svr_record_cmp(index_info, *last_record, record);
        if (r == 0) { /*已经找到 */
            memcpy(*last_record, record, svr->m_record_size);
            return 0;
        }
        else if (r < 0) { /*在当前块 */
            void ** begin = cur_buf->m_records;
            void ** end = last_record;

            while(begin != end) {
                void ** middle;
                int r;

                middle = begin + ((end - begin) / 2);
                assert(middle != end);
                r = rank_f_svr_record_cmp(index_info, *middle, record);
                if (r == 0) {
                    memcpy(*middle, record, svr->m_record_size);
                    return 0;
                }
                else if (r < 0) {
                    end = middle;
                }
                else {
                    begin = middle + 1;
                }
            }

            insert_buf = cur_buf;
            insert_pos = begin - cur_buf->m_records;
            break;
        }
        else {
            if (cur_buf->m_next == NULL) {
                if (cur_buf->m_record_count < RANK_F_SVR_INDEX_BUF_RECORD_COUNT) {
                    insert_buf = cur_buf;
                    insert_pos = cur_buf->m_record_count;
                }

                break;
            }
        }
    }

    place_record = aom_obj_alloc(svr->m_record_mgr);
    if (place_record == NULL) {
        CPE_ERROR(svr->m_em, "%s: record update: alloc new record fail!", rank_f_svr_name(svr));
        return -1;
    }
    memcpy(place_record, record, svr->m_record_size);

    /*在已有的块中插入*/
    while(place_record && insert_buf) {
        assert(insert_buf == *buf);

        if (insert_buf->m_record_count == RANK_F_SVR_INDEX_BUF_RECORD_COUNT) { /*当前块放满了 */
            void * save_record;

            assert(insert_pos < RANK_F_SVR_INDEX_BUF_RECORD_COUNT);

            save_record = insert_buf->m_records[insert_buf->m_record_count - 1];
            assert(save_record);

            if (insert_pos + 1 < insert_buf->m_record_count) {
                memmove(
                    insert_buf->m_records + insert_pos + 1,
                    insert_buf->m_records + insert_pos,
                    sizeof(insert_buf->m_records[0]) * (insert_buf->m_record_count - 1 - insert_pos));
            }

            insert_buf->m_records[insert_pos] = place_record;
            place_record = save_record;

            insert_pos = 0;
            insert_buf = insert_buf->m_next;

            buf = &(*buf)->m_next;
        }
        else {
            assert(insert_buf->m_record_count < RANK_F_SVR_INDEX_BUF_RECORD_COUNT);
            assert(insert_pos <= insert_buf->m_record_count);

            if (insert_pos < insert_buf->m_record_count) {
                memmove(
                    insert_buf->m_records + insert_pos + 1,
                    insert_buf->m_records + insert_pos,
                    sizeof(insert_buf->m_records[0]) * (insert_buf->m_record_count - insert_pos));
            }

            insert_buf->m_records[insert_pos] = place_record;
            place_record = NULL;

            ++insert_buf->m_record_count;
        }
    }

    if (place_record) {
        rank_f_svr_index_buf_t new_buf;

        assert(insert_buf == NULL);
        assert(buf);
        assert(*buf == NULL);
        assert(insert_pos == 0);

        new_buf = rank_f_svr_index_buf_alloc(svr);
        if (new_buf == NULL) {
            CPE_ERROR(svr->m_em, "%s: record update: alloc new buff fail!", rank_f_svr_name(svr));
            aom_obj_free(svr->m_record_mgr, place_record);
            return -1;
        }

        new_buf->m_records[0] = place_record;
        new_buf->m_record_count = 1;
        assert(new_buf->m_next == NULL);

        place_record = NULL;
        insert_buf = NULL;

        *buf = new_buf;
    }

    ++index->m_record_count;

    return 0;
}

int rank_f_svr_record_remove(rank_f_svr_t svr, rank_f_svr_index_t gid_index, void const * key) {
    rank_f_svr_index_info_t index_info;
    rank_f_svr_index_buf_t * buf;
    rank_f_svr_index_buf_t remove_in_buf;
    int remove_pos;

    assert(gid_index->m_index_id == 0);

    index_info = &svr->m_index_infos[gid_index->m_index_id];
    assert(index_info->m_svr);

    remove_pos = -1;

    for(buf = &gid_index->m_bufs; remove_pos == -1 && *buf; buf = &(*buf)->m_next) {
        rank_f_svr_index_buf_t cur_buf = *buf;
        void ** last_record;
        int r;

        assert(cur_buf->m_record_count > 0);

        last_record = cur_buf->m_records + cur_buf->m_record_count - 1;

        r = rank_f_svr_record_cmp(index_info, *last_record, key);
        if (r == 0) { /*已经找到 */
            remove_pos = last_record - cur_buf->m_records;
            break;
        }
        else if (r < 0) { /*在当前块 */
            void ** begin = cur_buf->m_records;
            void ** end = last_record;

            while(begin != end) {
                void ** middle;
                int r;

                middle = begin + ((end - begin) / 2);
                assert(middle != end);
                r = rank_f_svr_record_cmp(index_info, *middle, key);
                if (r == 0) {
                    remove_pos = middle - cur_buf->m_records;
                    break;
                }
                else if (r < 0) {
                    end = middle;
                }
                else {
                    begin = middle + 1;
                }
            }

            break;
        }
    }

    if (remove_pos == -1) {
        return SVR_RANK_F_ERROR_RECORD_NOT_EXIST;
    }

    assert(buf);
    assert(*buf);

    remove_in_buf = *buf;

    assert(remove_pos < remove_in_buf->m_record_count);

    aom_obj_free(svr->m_record_mgr, remove_in_buf->m_records[remove_pos]);

    if (remove_pos + 1 < remove_in_buf->m_record_count) {
        memmove(
            remove_in_buf->m_records + remove_pos,
            remove_in_buf->m_records + remove_pos + 1,
            sizeof(remove_in_buf->m_records[0]) * (remove_in_buf->m_record_count - remove_pos - 1));
    }
    remove_in_buf->m_record_count--;

    while(remove_in_buf->m_record_count < (RANK_F_SVR_INDEX_BUF_RECORD_COUNT / 2)) {
        assert(remove_in_buf == *buf);

        if (remove_in_buf->m_record_count == 0) {
            rank_f_svr_index_buf_t free_buf = remove_in_buf;

            remove_in_buf = free_buf->m_next;
            *buf = free_buf->m_next;
            free_buf->m_next = NULL;

            rank_f_svr_index_buf_free(svr, free_buf);
        }
        else if (remove_in_buf->m_next) {
            rank_f_svr_index_buf_t next_buf = remove_in_buf->m_next;        
            int move_count;

            move_count = RANK_F_SVR_INDEX_BUF_RECORD_COUNT - remove_in_buf->m_record_count;
            if (move_count > next_buf->m_record_count) {
                move_count = next_buf->m_record_count;
            }

            memmove(
                remove_in_buf->m_records + remove_in_buf->m_record_count,
                next_buf->m_records,
                sizeof(remove_in_buf->m_records[0]) * move_count);

            remove_in_buf->m_record_count += move_count;

            if (next_buf->m_record_count > move_count) {
                memmove(
                    next_buf->m_records,
                    next_buf->m_records + move_count,
                    sizeof(remove_in_buf->m_records[0]) * (next_buf->m_record_count - move_count));
            }

            next_buf->m_record_count -= move_count;

            buf = &remove_in_buf->m_next;
            remove_in_buf = next_buf;
        }
        else {
            break;
        }
    }

    gid_index->m_record_count--;

    return 0;
}

void *
rank_f_svr_record_find(rank_f_svr_t svr, rank_f_svr_index_t index, void const * key) {
    rank_f_svr_index_info_t index_info;
    rank_f_svr_index_buf_t buf;
    int record_count;

    index_info = &svr->m_index_infos[index->m_index_id];
    assert(index_info->m_svr);

    for(buf = index->m_bufs, record_count = index->m_record_count;
        buf && record_count > 0;
        buf = buf->m_next, record_count -= RANK_F_SVR_INDEX_BUF_RECORD_COUNT)
    {
        void ** last_record =
            buf->m_records
            + ((record_count > RANK_F_SVR_INDEX_BUF_RECORD_COUNT
               ? RANK_F_SVR_INDEX_BUF_RECORD_COUNT
               : record_count)
               - 1);
        int r = rank_f_svr_record_cmp(index_info, *last_record, key);
        if (r == 0) { /*已经找到 */
            return *last_record;
        }
        else if (r < 0) { /*在当前块 */
            void ** begin = buf->m_records;
            void ** end = last_record;

            while(begin != end) {
                void ** middle;
                int r;

                middle = begin + ((end - begin) / 2);
                assert(middle != end);
                r = rank_f_svr_record_cmp(index_info, *middle, key);
                if (r == 0) {
                    return middle;
                }
                else if (r < 0) {
                    end = middle;
                }
                else {
                    begin = middle + 1;
                }
            }

            return NULL;
        }
        else { /*在下一块 */
            //goto next buf
        }
    }

    return NULL;
}

static rank_f_svr_index_info_t g_index_info_for_sort;
static int rank_f_svr_record_qsort_cmp(void const * l, void const * r) {
    return rank_f_svr_record_cmp(g_index_info_for_sort, *(SVR_RANK_F_RECORD**)l, *(SVR_RANK_F_RECORD**)r);
}

int rank_f_svr_record_sort(rank_f_svr_t svr, rank_f_svr_index_t index, rank_f_svr_index_t records) {
    rank_f_svr_index_info_t index_info;
    rank_f_svr_index_buf_t buf;
    SVR_RANK_F_RECORD * sortbuf[records->m_record_count];
    uint8_t write_pos = 0;
    rank_f_svr_index_buf_t * link_to;

    assert(index->m_bufs == NULL);

    index_info = &svr->m_index_infos[index->m_index_id];
    assert(index_info->m_svr);

    /*将现有的record统一放入 */
    for(buf = records->m_bufs; buf; buf = buf->m_next) {
        uint8_t rp;
        for(rp = 0; rp < RANK_F_SVR_INDEX_BUF_RECORD_COUNT && write_pos < records->m_record_count; ++rp) {
            assert(buf->m_records[rp]);
            sortbuf[write_pos++] = buf->m_records[rp];
        }
    }

    /*排序 */
    g_index_info_for_sort = index_info;
    qsort(sortbuf, write_pos, sizeof(sortbuf[0]), rank_f_svr_record_qsort_cmp);

    /*将排序结果回填 */
    link_to = &index->m_bufs;
    while(index->m_record_count < records->m_record_count) {
        uint8_t i;

        buf = rank_f_svr_index_buf_alloc(svr);
        if (buf == NULL) {
            CPE_ERROR(svr->m_em, "%s: record sort: alloc new buf fail!", rank_f_svr_name(svr));
            return -1;
        }

        *link_to = buf;
        link_to = &buf->m_next;
        
        for(i = 0;
            i <  RANK_F_SVR_INDEX_BUF_RECORD_COUNT && index->m_record_count < records->m_record_count;
            ++i, ++index->m_record_count)
        {
            buf->m_records[i] = sortbuf[index->m_record_count];
        }
    }

    return 0;
}

