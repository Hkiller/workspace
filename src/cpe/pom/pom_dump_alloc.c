#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/bitarry.h"
#include "cpe/pom/pom_manage.h"
#include "pom_internal_ops.h"

static void pom_mgr_dump_alloc_class_info(write_stream_t stream, struct pom_class_mgr * class_mgr, int level) {
    size_t i;

    stream_putc_count(stream, ' ', level << 2);
    stream_printf(stream, "class info:\n");

    for(i = 0; i < (sizeof(class_mgr->m_classes) / sizeof(class_mgr->m_classes[0])); ++i) {
        struct pom_class * pom_class = &class_mgr->m_classes[i];
        size_t page_pos;

        stream_putc_count(stream, ' ', (level + 1) << 2);
        stream_printf(stream, "class [%d]: ", (int)i);

        if (pom_class->m_id == POM_INVALID_CLASSID) {
            stream_printf(stream, "INVALID\n");
            continue;
        }

        stream_printf(
            stream, "%s: object-size=%d, object-per-page=%d, page-size=%d, alloc-buf-capacity=%d, begin-in-page=%d\n",
            cpe_hs_data(pom_class->m_name),
            (int)pom_class->m_object_size,
            (int)pom_class->m_object_per_page,
            (int)pom_class->m_page_size,
            (int)pom_class->m_object_buf_begin_in_page);

        for(page_pos = 0; page_pos < pom_class->m_page_array_size; ++page_pos) {
            void * page_data = pom_class->m_page_array[page_pos];
            size_t obj_pos;
            cpe_ba_t alloc_arry = pom_class_ba_of_page(page_data);

            stream_putc_count(stream, ' ', (level + 2) << 2);
            stream_printf(stream, "page [%d]: ", (int)page_pos);
            if (page_data == NULL) {
                stream_printf(stream, "INVALID\n");
                continue;
            }

            stream_printf(stream, "address=%p: [", page_data);

            for(obj_pos = 0; obj_pos < pom_class->m_object_per_page; ++obj_pos) {
                if (obj_pos != 0) { stream_printf(stream, ", "); }
                stream_putc(stream, cpe_ba_get(alloc_arry, obj_pos) ? '1' : '0');
            }

            stream_printf(stream, "]\n");
        }
    }
}

static void pom_grp_dump_urange_it(write_stream_t stream, cpe_urange_it_t urange_it, int level) {
    int i = 0;
    struct cpe_urange urange;

    for(urange = cpe_urange_it_next(urange_it);
        cpe_urange_is_valid(urange); 
        ++i, urange = cpe_urange_it_next(urange_it))
    {
        if (i > 8) { stream_putc(stream, '\n'); i = 0; }

        if (i == 0) {
            stream_putc_count(stream, ' ', level << 2);
        }
        else {
            stream_printf(stream, ", ");
        }

        stream_printf(stream, "["FMT_PTR_INT_T", "FMT_PTR_INT_T")", urange.m_start, urange.m_end);
    }
}

static void pom_mgr_dump_alloc_buf_info(write_stream_t stream, struct pom_buffer_mgr * buf_mgr, int level) {
    struct cpe_urange_it urange_it;

    stream_putc_count(stream, ' ', level << 2);
    stream_printf(stream, "free pages:\n");
    cpe_urange_mgr_uranges(&urange_it, &buf_mgr->m_free_pages);
    pom_grp_dump_urange_it(stream, &urange_it, level + 1);

    stream_putc_count(stream, ' ', level << 2);
    stream_printf(stream, "buffers:\n");
    cpe_urange_mgr_uranges(&urange_it, &buf_mgr->m_buffers);
    pom_grp_dump_urange_it(stream, &urange_it, level + 1);

    stream_putc_count(stream, ' ', level << 2);
    stream_printf(stream, "buffer-ids:\n");
    cpe_urange_mgr_uranges(&urange_it, &buf_mgr->m_buffer_ids);
    pom_grp_dump_urange_it(stream, &urange_it, level + 1);
}

void pom_mgr_dump_alloc_info(write_stream_t stream, pom_mgr_t mgr, int level) {
    stream_putc_count(stream, ' ', level << 2);
    stream_printf(stream, "pom_mgr alloc info:\n");

    pom_mgr_dump_alloc_class_info(stream, &mgr->m_classMgr, level + 1);
    pom_mgr_dump_alloc_buf_info(stream, &mgr->m_bufMgr, level + 1);
}
