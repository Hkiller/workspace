#include <assert.h>
#if defined CPE_HAVE_EXECINFO_H
#include <execinfo.h>
#endif
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/bitarry.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/pom/pom_object.h"
#include "pom_internal_ops.h"
#include "pom_page_head.h"

static uint32_t pom_alloc_info_hash(const struct pom_alloc_info * context) {
    return (uint32_t)context->m_oid;
}

static int pom_alloc_info_cmp(const struct pom_alloc_info * l, const struct pom_alloc_info * r) {
    return l->m_oid == r->m_oid;
}

struct pom_debuger *
pom_debuger_create(mem_allocrator_t alloc, uint32_t stack_size, error_monitor_t em) {
    struct pom_debuger * debuger;

    debuger = mem_alloc(alloc, sizeof(struct pom_debuger));
    if (debuger == NULL) return NULL;

    debuger->m_alloc = alloc;
    debuger->m_em = em;
    debuger->m_stack_size = stack_size;

    if (cpe_hash_table_init(
            &debuger->m_alloc_infos,
            alloc,
            (cpe_hash_fun_t)pom_alloc_info_hash,
            (cpe_hash_eq_t)pom_alloc_info_cmp,
            CPE_HASH_OBJ2ENTRY(pom_alloc_info, m_hh),
            0) != 0)
    {
        CPE_ERROR(em, "pom_debuger: create: init hash table fail!");
        mem_free(alloc, debuger);
        return NULL;
    }

    return debuger;
}

void pom_debuger_free(struct pom_debuger * debuger) {
    struct cpe_hash_it debug_info_it;
    struct pom_debug_info * debug_info;

    assert(debuger);

    cpe_hash_it_init(&debug_info_it, &debuger->m_alloc_infos);

    debug_info = cpe_hash_it_next(&debug_info_it);
    while (debug_info) {
        struct pom_debug_info * next = cpe_hash_it_next(&debug_info_it);

        cpe_hash_table_remove_by_ins(&debuger->m_alloc_infos, debug_info);
        mem_free(debuger->m_alloc, debug_info);

        debug_info = next;
    }

    cpe_hash_table_fini(&debuger->m_alloc_infos);

    mem_free(debuger->m_alloc, debuger);
}

static void pom_debuger_dump_stack(write_stream_t stream, struct pom_alloc_info * alloc_info, int ident) {
    char ** symbols;
#if defined CPE_HAVE_EXECINFO_H
    symbols = backtrace_symbols((void **)(alloc_info + 1), alloc_info->m_stack_size);
#else
    symbols = NULL;
#endif
    if (symbols) {
        int i;
        for(i = 0; i < alloc_info->m_stack_size; ++i) {
            stream_putc_count(stream, ' ', ident);
            stream_printf(stream, "%s\n",  symbols[i]);
        }
        free(symbols);
    }
    else {
        stream_putc_count(stream, ' ', ident);
        stream_printf(stream, "no stack");
    }
}

void pom_debuger_on_alloc(struct pom_debuger * debuger, pom_oid_t oid) {
    struct pom_alloc_info * alloc_info;
    struct pom_alloc_info * old_alloc_info;

    alloc_info =
        (struct pom_alloc_info *)mem_alloc(
            debuger->m_alloc,
            sizeof(struct pom_alloc_info) + sizeof(void*) * debuger->m_stack_size);
    if (alloc_info == NULL) {
        CPE_ERROR(debuger->m_em, "pom_debuger: alloc: save alloc info fail, no using memory!");
        return;
    }

    alloc_info->m_oid = oid;
    alloc_info->m_free = 0;
#if defined CPE_HAVE_EXECINFO_H
    alloc_info->m_stack_size = backtrace((void**)(alloc_info + 1), debuger->m_stack_size);
#else
    alloc_info->m_stack_size = 0;
#endif
    cpe_hash_entry_init(&alloc_info->m_hh);

    old_alloc_info = cpe_hash_table_find(&debuger->m_alloc_infos, alloc_info);
    if (old_alloc_info) {
        if (!old_alloc_info->m_free) {
            struct mem_buffer print_buffer;
            struct write_stream_buffer stream;

            mem_buffer_init(&print_buffer, debuger->m_alloc);
            write_stream_buffer_init(&stream, &print_buffer);

            stream_printf((write_stream_t)&stream, "oid %d already allocked\n", oid);
            stream_printf((write_stream_t)&stream, "    now allock from:\n");
            pom_debuger_dump_stack((write_stream_t)&stream, alloc_info, 8);

            stream_printf((write_stream_t)&stream, "    old allock from:\n");
            pom_debuger_dump_stack((write_stream_t)&stream, old_alloc_info, 8);
        
            CPE_ERROR(debuger->m_em, "pom_debuger: alloc: %s", (char*)mem_buffer_make_continuous(&print_buffer, 0));

            mem_buffer_clear(&print_buffer);

            assert(0 || "pom_alloc: alloc a allocked oid!"); 
        }

        cpe_hash_table_remove_by_ins(&debuger->m_alloc_infos, old_alloc_info);
        mem_free(debuger->m_alloc, old_alloc_info);
        old_alloc_info = NULL;
    }

    if (cpe_hash_table_insert_unique(&debuger->m_alloc_infos, alloc_info) != 0) {
        assert(0 || "pom_alloc: insert fail!!! why???");
        mem_free(debuger->m_alloc, alloc_info);
    }
}

void pom_debuger_on_free(struct pom_debuger * debuger, pom_oid_t oid) {
    struct pom_alloc_info * alloc_info;
    struct pom_alloc_info key;

    key.m_oid = oid;

    alloc_info = cpe_hash_table_find(&debuger->m_alloc_infos, &key);
    if (alloc_info == NULL) {
        CPE_ERROR(debuger->m_em, "pom_debuger: free: oid %d not allocked!", oid);
        assert(0 || "pom_free: free a not allocked oid");
    }
    else if (alloc_info->m_free) {
        struct mem_buffer print_buffer;
        struct write_stream_buffer stream;

        mem_buffer_init(&print_buffer, debuger->m_alloc);
        write_stream_buffer_init(&stream, &print_buffer);

        stream_printf((write_stream_t)&stream, "oid %d already free\n", oid);
        stream_printf((write_stream_t)&stream, "    free from:\n");
        pom_debuger_dump_stack((write_stream_t)&stream, alloc_info, 8);

        CPE_ERROR(debuger->m_em, "pom_debuger: free: %s", (char*)mem_buffer_make_continuous(&print_buffer, 0));

        mem_buffer_clear(&print_buffer);

        assert(0 || "pom_free: free a not allocked oid");
    }
    else {
        alloc_info->m_free = 1;
#if defined CPE_HAVE_EXECINFO_H
        alloc_info->m_stack_size = backtrace((void**)(alloc_info + 1), debuger->m_stack_size);
#else
        alloc_info->m_stack_size = 0;
#endif
    }
}

void pom_debuger_restore_one_page(struct pom_debuger * debuger, void * page_data) {
    struct pom_data_page_head * page = page_data;
    size_t obj_pos;
    cpe_ba_t alloc_arry = pom_class_ba_of_page(page_data);

    if (page->m_classId == POM_INVALID_CLASSID) return;

    for(obj_pos = 0; obj_pos < page->m_obj_per_page; ++obj_pos) {
        if (cpe_ba_get(alloc_arry, obj_pos) == cpe_ba_true) {
            pom_oid_t base_oid = (pom_oid_t)(page->m_obj_per_page * page->m_page_idx + obj_pos);
            pom_debuger_on_alloc(debuger, pom_oid_make(page->m_classId, base_oid));
        }
    }
}
