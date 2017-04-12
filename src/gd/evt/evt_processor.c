#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "gd/evt/evt_manage.h"
#include "evt_internal_ops.h"

int gd_evt_processor_free_id(gd_evt_mgr_t mgr, evt_processor_id_t processorId) {
    return cpe_range_put_one(&mgr->m_ids, processorId);
}

struct gd_evt_processor *
gd_evt_processor_get(gd_evt_mgr_t mgr, evt_processor_id_t processorId) {
    size_t pagePos;
    struct gd_evt_processor * processorPage;

    pagePos  = processorId / mgr->m_processor_count_in_page;
    assert(pagePos < mgr->m_processor_page_count);

    processorPage  = mgr->m_processor_buf[pagePos];
    return &processorPage[processorId % mgr->m_processor_count_in_page];
}

int gd_evt_processor_alloc(gd_evt_mgr_t mgr, evt_processor_id_t * id) {
    ptr_int_t newStart;
    struct gd_evt_processor * newPage;
    size_t i;

    if (!cpe_range_mgr_is_empty(&mgr->m_ids)) {
        *id = (evt_processor_id_t)cpe_range_get_one(&mgr->m_ids);
        return 0;
    }

    if (mgr->m_processor_page_count + 1 >= mgr->m_processor_page_capacity) {
        size_t newProcessorPageCapacity;
        struct gd_evt_processor ** newProcessorBuf;

        newProcessorPageCapacity = mgr->m_processor_page_count + 128;
        newProcessorBuf = (struct gd_evt_processor **)mem_alloc(mgr->m_alloc, sizeof(struct gd_evt_processor*) * newProcessorPageCapacity);
        if (newProcessorBuf == NULL) return -1;

        bzero(newProcessorBuf, sizeof(struct gd_evt_processor *) * newProcessorPageCapacity);
        memcpy(newProcessorBuf, mgr->m_processor_buf, sizeof(struct gd_evt_processor*) * mgr->m_processor_page_count);

        if (mgr->m_processor_buf) mem_free(mgr->m_alloc, mgr->m_processor_buf);

        mgr->m_processor_buf = newProcessorBuf;
        mgr->m_processor_page_capacity = newProcessorPageCapacity;

        if (mgr->m_debug) {
            CPE_INFO(mgr->m_em, "%s: gd_evt_processor_alloc: resize processor buf to "  FMT_SIZE_T, gd_evt_mgr_name(mgr), newProcessorPageCapacity);
        }
    }

    newStart = mgr->m_processor_page_count * mgr->m_processor_count_in_page;
    newPage = (struct gd_evt_processor *)mem_alloc(mgr->m_alloc, sizeof(struct gd_evt_processor) * mgr->m_processor_count_in_page);
    if (newPage == NULL) {
        return -1;
    }

    bzero(newPage, sizeof(struct gd_evt_processor) * mgr->m_processor_count_in_page);
    for(i = 0; i < mgr->m_processor_count_in_page; ++i) {
        newPage[i].m_id = (evt_processor_id_t)(newStart + i);
    }

    if (cpe_range_put_range(&mgr->m_ids, newStart, newStart + mgr->m_processor_count_in_page) != 0) {
        mem_free(mgr->m_alloc, newPage);
        return -1;
    }

    mgr->m_processor_buf[mgr->m_processor_page_count] = newPage;
    ++mgr->m_processor_page_count;

    if (mgr->m_debug) {
        CPE_INFO(
            mgr->m_em,
            "alloc a new processor page[%d,%d), page count is "  FMT_SIZE_T,
            (evt_processor_id_t)newStart, (evt_processor_id_t)(newStart + mgr->m_processor_page_count),
            mgr->m_processor_page_count);
    }

    *id = (evt_processor_id_t)cpe_range_get_one(&mgr->m_ids);
    return 0;
}

void gd_evt_mgr_free_processor_buf(gd_evt_mgr_t mgr) {
    size_t i;

    for(i = 0; i < mgr->m_processor_page_count; ++i) {
        mem_free(mgr->m_alloc, mgr->m_processor_buf[i]);
    }

    if (mgr->m_processor_buf)
        mem_free(mgr->m_alloc, mgr->m_processor_buf);

    mgr->m_processor_page_count = 0;
    mgr->m_processor_page_capacity = 0;
}

void gd_evt_processor_free_basic(gd_evt_mgr_t mgr, struct gd_evt_processor * data) {
    if (data->m_state == evt_processor_state_InResponserHash) {
        cpe_hash_table_remove_by_ins(&mgr->m_responser_to_processor, data);
        data->m_state = evt_processor_state_NotInResponserHash;
    }

    if (data->m_process_arg_free) data->m_process_arg_free(data->m_process_arg);

    data->m_process_ctx = NULL;
    data->m_process_arg = NULL;
    data->m_process_arg_free = NULL;
    data->m_process_fun = NULL;
}

uint32_t gd_evt_processor_hash_fun(const struct gd_evt_processor * o) {
    return (((ptr_int_t)o->m_process_ctx) & 0xFFFFFFFF);
}

int gd_evt_processor_cmp_fun(const struct gd_evt_processor * l, const struct gd_evt_processor * r) {
    return l->m_process_ctx == r->m_process_ctx;
}
