#include <stdlib.h>
#include <assert.h>
#if defined CPE_HAVE_EXECINFO_H
#include <execinfo.h>
#endif
#include "cpe/pal/pal_stdio.h"
#include "timer_internal_ops.h"

#ifdef CPE_TIMER_DEBUG

#define CPE_TIMER_STACK_SIZE (10)

uint32_t cpe_debug_info_hash_fun(const struct cpe_timer_alloc_info * o) {
    return o->m_timer_id;
}

int cpe_debug_info_eq_fun(const struct cpe_timer_alloc_info * l, const struct cpe_timer_alloc_info * r) {
    return l->m_timer_id == r->m_timer_id;
}

void cpe_alloc_info_add(cpe_timer_mgr_t mgr, cpe_timer_id_t id) {
    struct cpe_timer_alloc_info * alloc_info;
    struct cpe_timer_alloc_info key;

    key.m_timer_id = id;
    alloc_info = cpe_hash_table_find(&mgr->m_alloc_infos, &key);
    if (alloc_info == NULL) {
        size_t alloc_size = sizeof(struct cpe_timer_alloc_info);

#if defined CPE_HAVE_EXECINFO_H
        alloc_size += sizeof(void*) * CPE_TIMER_STACK_SIZE;
#endif

        alloc_info = mem_alloc(mgr->m_alloc, alloc_size);
        assert(alloc_info);
        if (alloc_info == NULL) return;

        alloc_info->m_timer_id = id;
        alloc_info->m_is_free = 1;
        alloc_info->m_stack_size = 0;
        cpe_hash_entry_init(&alloc_info->m_hh);

        if (cpe_hash_table_insert_unique(&mgr->m_alloc_infos, alloc_info) != 0) {
            assert(0);
        }
    }

    if (alloc_info->m_is_free != 1) {
        fprintf(stderr, "timer id %d is already allocked\n", id);
#if defined CPE_HAVE_EXECINFO_H
        fprintf(stderr, "   alloc from\n");
        if (alloc_info->m_stack_size) {
            char ** symbols;
            symbols = backtrace_symbols((void **)(alloc_info + 1), alloc_info->m_stack_size);
            if (symbols) {
                int i;
                for(i = 0; i < alloc_info->m_stack_size; ++i) {
                    fprintf(stderr, "        %s\n", symbols[i]);
                }
                free(symbols);
            }
        } while(0);
#endif
        assert(0 && "timer already allocked !!!");
        return;
    }

    assert(alloc_info->m_timer_id == id);
    alloc_info->m_is_free = 0;

#if defined CPE_HAVE_EXECINFO_H
    alloc_info->m_stack_size = backtrace((void**)(alloc_info + 1), CPE_TIMER_STACK_SIZE);
#endif


}

void cpe_alloc_info_remove(cpe_timer_mgr_t mgr, cpe_timer_id_t id) {
    struct cpe_timer_alloc_info * alloc_info;
    struct cpe_timer_alloc_info key;

    key.m_timer_id = id;
    alloc_info = cpe_hash_table_find(&mgr->m_alloc_infos, &key);
    if (alloc_info == NULL) {
        fprintf(stderr, "timer %d not allocked!", id);
        assert(0 && "timer not allocked !!!");
        return;
    }    

    if (alloc_info->m_is_free == 1) {
        fprintf(stderr, "timer id %d is already removed\n", id);
#if defined CPE_HAVE_EXECINFO_H
        fprintf(stderr, "   remove from\n");
        if (alloc_info->m_stack_size) {
            char ** symbols;
            symbols = backtrace_symbols((void **)(alloc_info + 1), alloc_info->m_stack_size);
            if (symbols) {
                int i;
                for(i = 0; i < alloc_info->m_stack_size; ++i) {
                    fprintf(stderr, "        %s\n", symbols[i]);
                }
                free(symbols);
            }
        } while(0);
#endif
        assert(0 && "timer already removed !!!");
        return;
    }

    assert(alloc_info->m_timer_id == id);
    alloc_info->m_is_free = 1;

#if defined CPE_HAVE_EXECINFO_H
    alloc_info->m_stack_size = backtrace((void**)(alloc_info + 1), CPE_TIMER_STACK_SIZE);
#endif
}

#endif
