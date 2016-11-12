#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/bitarry.h"
#include "cpe/pom/pom_manage.h"
#include "pom_internal_ops.h"

static void pom_mgr_dump_page_info_one_buf(write_stream_t stream, char const * buf, size_t size, size_t page_size, int level) {
    cpe_ba_t alloc_arry;
    int i;

    while(size >= page_size) {
        struct pom_data_page_head * head = (struct pom_data_page_head *)buf;

        stream_putc_count(stream, ' ', level << 2);

        stream_printf(
            stream, "page (class=%d, index=%d, obj-per-page=%d): %p ~ %p\n",
            head->m_classId, head->m_page_idx, head->m_obj_per_page,
            ((char*)(head + 1)) + cpe_ba_bytes_from_bits(head->m_obj_per_page), buf + page_size);

        alloc_arry = pom_class_ba_of_page(buf);

        stream_putc_count(stream, ' ', (level + 1) << 2);
        for(i = 0; i < head->m_obj_per_page; ++i) {
            stream_putc(stream, cpe_ba_get(alloc_arry, i) ? '1' : '0');
        }
        stream_putc(stream, '\n');

        buf += page_size;
        size -= page_size;
    }
}

void pom_mgr_dump_page_info(write_stream_t stream, pom_mgr_t mgr, int level) {
    struct cpe_urange_it buf_urange_it;
    struct cpe_urange buf_urange;
    struct pom_buffer_mgr * buf_mgr;

    buf_mgr = &mgr->m_bufMgr;

    cpe_urange_mgr_uranges(&buf_urange_it, &buf_mgr->m_buffer_ids);

    for(buf_urange = cpe_urange_it_next(&buf_urange_it);
        cpe_urange_is_valid(buf_urange);
        buf_urange = cpe_urange_it_next(&buf_urange_it))
    {
        for(; buf_urange.m_start != buf_urange.m_end; ++buf_urange.m_start) {
            char * page_buf = (char *)pom_buffer_mgr_get_buf(buf_mgr, buf_urange.m_start, NULL);
            stream_putc_count(stream, ' ', level << 2);
            if (page_buf == NULL) {
                stream_printf(stream, "buf "FMT_UINT64_T": no data\n", buf_urange.m_start);
                continue;
            }
            else {
                stream_printf(
                    stream, "buf "FMT_UINT64_T": %p ~ %p (size="FMT_SIZE_T")\n",
                    buf_urange.m_start, page_buf, page_buf + buf_mgr->m_buf_size, buf_mgr->m_buf_size);

                pom_mgr_dump_page_info_one_buf(
                    stream, 
                    page_buf,
                    buf_mgr->m_buf_size,
                    buf_mgr->m_page_size,
                    level + 1);
            }
        }
    }
}
