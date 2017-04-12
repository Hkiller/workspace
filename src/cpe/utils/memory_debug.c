#include <assert.h>
#if defined CPE_HAVE_EXECINFO_H
#include <execinfo.h>
#endif
#include <cpe/pal/pal_stdlib.h>
#include <cpe/pal/pal_strings.h>
#include "cpe/utils/memory_debug.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"

struct debug_mem_alloc_info {
    void * m_addr;
    size_t m_size;
    int m_stack_size;
    struct cpe_hash_entry m_hh;
};

struct debug_mem_allocrator {
    struct mem_allocrator m_alloc;
    mem_allocrator_t m_using;
    mem_allocrator_t m_parent;
    error_monitor_t m_em;
    int m_stack_size;
    int m_alloc_count;
    int m_alloc_size;
    int m_free_count;
    int m_free_size;
    struct cpe_hash_table m_alloc_infos;
};

static uint32_t debug_mem_alloc_info_hash(const struct debug_mem_alloc_info * context) {
#if (__WORDSIZE == 64)
    uint64_t value = (uint64_t)context->m_addr;
    return (uint32_t)(value & 0xFFFFFFFF);
#else
    return (uint32_t)context->m_addr;
#endif
}

static int debug_mem_alloc_info_cmp(const struct debug_mem_alloc_info * l, const struct debug_mem_alloc_info * r) {
    return l->m_addr == r->m_addr;
}

static void * do_debug_allocrator_alloc(size_t size, struct mem_allocrator * allocrator) {
    struct debug_mem_alloc_info * alloc_info;
    struct debug_mem_allocrator * dalloc;
    void * r;

    dalloc = (struct debug_mem_allocrator *)allocrator;

    r = mem_alloc(dalloc->m_parent, size);
    if (r == NULL) return NULL;

    dalloc->m_alloc_count++;
    dalloc->m_alloc_size += (int)size;

    alloc_info =
        (struct debug_mem_alloc_info *)mem_alloc(
            dalloc->m_using,
            sizeof(struct debug_mem_alloc_info) + sizeof(void*) * dalloc->m_stack_size);
    if (alloc_info == NULL) {
        CPE_ERROR(dalloc->m_em, "mem_alloc_debug: alloc: save alloc info fail, no using memory!");
        return r;
    }

    alloc_info->m_addr = r;
    alloc_info->m_size = size;
#if defined CPE_HAVE_EXECINFO_H
    alloc_info->m_stack_size = backtrace((void**)(alloc_info + 1), dalloc->m_stack_size);
#endif
    cpe_hash_entry_init(&alloc_info->m_hh);

    if (cpe_hash_table_insert_unique(&dalloc->m_alloc_infos, alloc_info) != 0) {
        assert(0);
        CPE_ERROR(dalloc->m_em, "mem_alloc_debug: alloc: parent alloc same address?!");
        mem_free(dalloc->m_using, alloc_info);
        return r;
    }

    return r;
}

static void * do_debug_allocrator_calloc(size_t size, struct mem_allocrator * allocrator) {
    void * r = do_debug_allocrator_alloc(size, allocrator);
    if (r) bzero(r, size);
    return r;
}

static void do_debug_allocrator_free(void * p, struct mem_allocrator * allocrator) {
    struct debug_mem_alloc_info key;
    struct debug_mem_alloc_info * alloc_info;
    struct debug_mem_allocrator * dalloc = (struct debug_mem_allocrator *)allocrator;

    key.m_addr = p;
    alloc_info = (struct debug_mem_alloc_info *)cpe_hash_table_find(&dalloc->m_alloc_infos, &key);
    if (alloc_info == NULL) {
        assert(0);
        CPE_ERROR(dalloc->m_em, "mem_alloc_debug: free: address %p not alloc from here or already released?!", p);
        return;
    }

    dalloc->m_free_count++;
    dalloc->m_free_size += alloc_info->m_size;

    cpe_hash_table_remove_by_ins(&dalloc->m_alloc_infos, alloc_info);
    mem_free(dalloc->m_using, alloc_info);

    mem_free(dalloc->m_parent, p);
}

mem_allocrator_t
mem_allocrator_debug_create(mem_allocrator_t using, mem_allocrator_t parent, int stack_size, error_monitor_t em) {
    struct debug_mem_allocrator * alloc;

    if (stack_size < 0) {
        CPE_ERROR(em, "mem_alloc_debug: create allocrator: stack size %d error!", stack_size);
        return NULL;
    }

    alloc = (struct debug_mem_allocrator *)mem_alloc(using, sizeof(struct debug_mem_allocrator));
    if (alloc == NULL) {
        CPE_ERROR(em, "mem_alloc_debug: create allocrator: alloc from parent fail!");
        return NULL;
    }

    alloc->m_alloc.m_alloc = do_debug_allocrator_alloc;
    alloc->m_alloc.m_calloc = do_debug_allocrator_calloc;
    alloc->m_alloc.m_free = do_debug_allocrator_free;
    alloc->m_using = using;
    alloc->m_parent = parent;
    alloc->m_stack_size = stack_size;
    alloc->m_em = em;
    alloc->m_alloc_size = 0;
    alloc->m_alloc_count = 0;
    alloc->m_free_size = 0;
    alloc->m_free_count = 0;

    if (cpe_hash_table_init(
            &alloc->m_alloc_infos,
            using,
            (cpe_hash_fun_t)debug_mem_alloc_info_hash,
            (cpe_hash_eq_t)debug_mem_alloc_info_cmp,
            CPE_HASH_OBJ2ENTRY(debug_mem_alloc_info, m_hh),
            0) != 0)
    {
        CPE_ERROR(em, "mem_alloc_debug: create allocrator: init hash table fail!");
        mem_free(parent, alloc);
        return NULL;
    }

    return (mem_allocrator_t)alloc;
}

static void mem_allocrator_debug_clear_infos(mem_allocrator_t allocrator) {
    struct cpe_hash_it it;
    struct debug_mem_alloc_info * info;
    struct debug_mem_allocrator * dalloc = (struct debug_mem_allocrator *)allocrator;

    cpe_hash_it_init(&it, &dalloc->m_alloc_infos);
    while((info = (struct debug_mem_alloc_info *)cpe_hash_it_next(&it))) {
        struct debug_mem_alloc_info * next = (struct debug_mem_alloc_info *)cpe_hash_it_next(&it);

        cpe_hash_table_remove_by_ins(&dalloc->m_alloc_infos, info);
        mem_free(dalloc->m_using, info);

        info = next; 
    }
}

void mem_allocrator_debug_check_dump(mem_allocrator_t allocrator) {
    struct mem_buffer buffer;
    struct write_stream_buffer stream;
    struct debug_mem_allocrator * dalloc = (struct debug_mem_allocrator *)allocrator;

    if (cpe_hash_table_count(&dalloc->m_alloc_infos) == 0) return;

    if (dalloc->m_em) {
        mem_buffer_init(&buffer, dalloc->m_using);
        write_stream_buffer_init(&stream, &buffer);

        stream_printf((write_stream_t)&stream, "mem_alloc_debug: mem leak found!\n");
        stream_printf((write_stream_t)&stream, "    alloc count: %d\n", dalloc->m_alloc_count);
        stream_printf((write_stream_t)&stream, "    free count: %d\n", dalloc->m_free_count);
        stream_printf((write_stream_t)&stream, "    alloc size: %d\n", dalloc->m_alloc_size);
        stream_printf((write_stream_t)&stream, "    free size: %d\n", dalloc->m_free_size);
        stream_printf((write_stream_t)&stream, "    alloc infos:\n");

        mem_allocrator_debug_dump((write_stream_t)&stream, 8, allocrator);

        mem_buffer_clear(&buffer);
    }
}

void mem_allocrator_debug_free(mem_allocrator_t allocrator) {
    struct debug_mem_allocrator * dalloc = (struct debug_mem_allocrator *)allocrator;

    mem_allocrator_debug_check_dump(allocrator);

    mem_allocrator_debug_clear_infos(allocrator);

    cpe_hash_table_fini(&dalloc->m_alloc_infos);

    mem_free(dalloc->m_using, dalloc);
}

void mem_allocrator_debug_dump(write_stream_t stream, int ident, mem_allocrator_t allocrator) {
    struct cpe_hash_it it;
    struct debug_mem_alloc_info * info;
    struct debug_mem_allocrator * dalloc = (struct debug_mem_allocrator *)allocrator;

    cpe_hash_it_init(&it, &dalloc->m_alloc_infos);
    while((info = (struct debug_mem_alloc_info *)cpe_hash_it_next(&it))) {
        stream_putc_count(stream, ' ', ident);
        stream_printf(stream, "address: %p, size: %d\n", info->m_addr, info->m_size);

#if defined CPE_HAVE_EXECINFO_H
        if (info->m_stack_size) {
            char ** symbols = backtrace_symbols((void **)(info + 1), info->m_stack_size);
            if (symbols) {
                int i;
                for(i = 0; i < info->m_stack_size; ++i) {
                    stream_putc_count(stream, ' ', ident + 4);
                    stream_printf(stream, "%s\n", symbols[i]);
                }
                free(symbols);
            }
        }
#endif

        stream_printf(stream, "\n");
    }
}

int mem_allocrator_debug_alloc_count(mem_allocrator_t dalloc) {
    return ((struct debug_mem_allocrator *)dalloc)->m_alloc_count;
}

int mem_allocrator_debug_free_count(mem_allocrator_t dalloc) {
    return ((struct debug_mem_allocrator *)dalloc)->m_free_count;
}

int mem_allocrator_debug_alloc_size(mem_allocrator_t dalloc) {
    return ((struct debug_mem_allocrator *)dalloc)->m_alloc_size;
}

int mem_allocrator_debug_free_size(mem_allocrator_t dalloc) {
    return ((struct debug_mem_allocrator *)dalloc)->m_free_size;
}
