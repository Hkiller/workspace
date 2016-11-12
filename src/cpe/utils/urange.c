#include <assert.h>
#include <stdio.h>
#include <string.h>
#if defined CPE_HAVE_EXECINFO_H
#include <execinfo.h>
#endif
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/range.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/stream_file.h"

static void cpe_urange_alloc_info_add(cpe_urange_mgr_t ra, ptr_uint_t id);
static void cpe_urange_alloc_info_put(cpe_urange_mgr_t ra, ptr_uint_t id);

int cpe_urange_mgr_init(cpe_urange_mgr_t ra, mem_allocrator_t alloc) {
    if (ra == NULL) return 0;

    ra->m_alloc = alloc;
    ra->m_urange_capacity = 0;
    ra->m_urange_count = 0;
    ra->m_uranges = NULL;
    ra->m_debug_info = NULL;

    return 0;
}

void cpe_urange_mgr_fini(cpe_urange_mgr_t ra) {
    assert(ra);

    if (ra->m_debug_info) {
        cpe_urange_set_debug(ra, 0);
        assert(ra->m_debug_info == NULL);
    }

    if (ra->m_uranges) {
        mem_free(ra->m_alloc, ra->m_uranges);
        ra->m_uranges = NULL;
    }

    ra->m_urange_count = 0;
    ra->m_urange_capacity = 0;
    ra->m_alloc = NULL;
}

void cpe_urange_mgr_clear(cpe_urange_mgr_t ra) {
    assert(ra);
    ra->m_urange_capacity = 0;
}

int cpe_urange_mgr_is_empty(cpe_urange_mgr_t ra) {
    return ra->m_urange_count == 0 ? 1 : 0;
}

void cpe_urange_mgr_dump(write_stream_t stream, cpe_urange_mgr_t ra) {
    size_t i;
    for(i = 0; i < ra->m_urange_count; ++i) {
        if (i > 0) stream_putc(stream, ',');

        stream_printf(
            stream, "[%d~%d)",
            ra->m_uranges[i].m_start,
            ra->m_uranges[i].m_end);
    }
}

int cpe_urange_get_one(cpe_urange_mgr_t ra, ptr_uint_t * value) {
    struct cpe_urange r;

    assert(ra);

    r = cpe_urange_get_urange(ra, 1);

    if (cpe_urange_is_valid(r)) {
        if (value) *value = r.m_start;
        return 0;
    }
    else {
        return -1;
    }
}

int cpe_urange_put_one(cpe_urange_mgr_t ra, ptr_uint_t value) {
    return cpe_urange_put_urange(ra, value, value + 1);
}

struct cpe_urange
cpe_urange_get_urange(cpe_urange_mgr_t ra, size_t capacity) {
    struct cpe_urange r;

    assert(ra);

    if (ra->m_urange_count == 0 || capacity <= 0) {
        r.m_start = 1;
        r.m_end = 0;
    }
    else {
        struct cpe_urange * allocFrom = ra->m_uranges;
        assert(allocFrom);

        r.m_start = allocFrom->m_start;
        if (allocFrom->m_end > allocFrom->m_start + (ptr_uint_t)capacity) {
            r.m_end = r.m_start + capacity;
            allocFrom->m_start += capacity;
        }
        else {
            r.m_end = allocFrom->m_end;
            --ra->m_urange_count;
            if (ra->m_urange_count > 0) {
                memmove(ra->m_uranges, ra->m_uranges + 1, sizeof(struct cpe_urange) * ra->m_urange_count);
            }
        }
    }

    if (ra->m_debug_info) {
        ptr_uint_t v = r.m_start;
        while(v != r.m_end) {
            cpe_urange_alloc_info_add(ra, v);
            ++v;
        }
    }

    return r;
}

static void cpe_urange_merge_neighbers(cpe_urange_mgr_t ra, int beginPos) {
    int keepPos;
    int removeCount;
    int checkPos;
    struct cpe_urange * curRange;

    keepPos = beginPos;
    removeCount = 0;

    curRange = ra->m_uranges + beginPos;

    if (beginPos > 0) {
        struct cpe_urange * preRange = curRange - 1;
        if (preRange->m_end >= curRange->m_start) {
            preRange->m_end = curRange->m_end;
            curRange = preRange;
            ++removeCount;
            --keepPos;
        }
    }

    for(checkPos = beginPos + 1; (size_t)checkPos < ra->m_urange_count; ++checkPos, ++removeCount) {
        struct cpe_urange * nextRange = ra->m_uranges + checkPos;
        if (nextRange->m_start <= curRange->m_end) {
            if (nextRange->m_end > curRange->m_end) {
                curRange->m_end = nextRange->m_end;
            }
        }
        else {
            break;
        }
    }

    if (removeCount > 0) {
        ra->m_urange_count -= removeCount;
        if ((size_t)(keepPos + 1) < ra->m_urange_count) {
            memmove(
                ra->m_uranges + keepPos + 1,
                ra->m_uranges + keepPos + 1 + removeCount,
                sizeof(struct cpe_urange) * (ra->m_urange_count - (keepPos + 1)));
        }
    }
}

static int cpe_urange_find_next_pos(cpe_urange_mgr_t ra, ptr_uint_t start) {
    struct cpe_urange * curRange;
    int beginPos, endPos, curPos;

    for(beginPos = 0, endPos = (int)ra->m_urange_count, curPos = (endPos - beginPos - 1) / 2;
        beginPos < endPos;
        curPos = beginPos + (endPos - beginPos - 1) / 2)
    {
        curRange = ra->m_uranges + curPos;
            
        if (curRange->m_start < start) {
            beginPos = curPos + 1;
        }
        else if (curRange->m_start > start) {
            endPos = curPos;
        }
        else {
            break;
        }
    }

    return ((size_t)curPos < ra->m_urange_count) && (ra->m_uranges[curPos].m_start >= start)
        ? curPos
        : (int)ra->m_urange_count;
}

int cpe_urange_is_conflict(cpe_urange_mgr_t ra, ptr_uint_t start, ptr_uint_t end) {
    int next_pos;
    int pre_pos;
    struct cpe_urange * next_urange;
    struct cpe_urange * pre_urange;

    assert(ra);

    if (end <= 0 || end < start) return 0;
    if (end == start) return 0;
    if (ra->m_urange_count == 0) return 0;

    next_pos = cpe_urange_find_next_pos(ra, start);
    pre_pos = next_pos - 1;

    next_urange = next_pos < (int)ra->m_urange_count ? ra->m_uranges + next_pos : NULL;
    pre_urange = (pre_pos >= 0 && pre_pos < (int)ra->m_urange_count) ? ra->m_uranges + pre_pos : NULL;

    if (next_urange && end > next_urange->m_start) return 1;
    if (pre_urange && pre_urange->m_end > start) return 1;

    return 0;
}

int cpe_urange_put_urange(cpe_urange_mgr_t ra, ptr_uint_t start, ptr_uint_t end) {
    if (end <= 0 || end < start) return -1;
    if (end == start) return 0;

    assert(ra);

    if (ra->m_urange_count == 0) {
        if (cpe_urange_mgr_reserve_for_put(ra, 1) != 0) return -1;
        assert(ra->m_uranges);
        ra->m_uranges[0].m_start = start;
        ra->m_uranges[0].m_end = end;
        ++ra->m_urange_count;

        if (ra->m_debug_info) {
            ptr_uint_t v = start;
            while(v != end) {
                cpe_urange_alloc_info_put(ra, v);
                ++v;
            }
        }

        return 0;
    }
    else {
        int insertPos = cpe_urange_find_next_pos(ra, start);

        if (insertPos < (int)ra->m_urange_count) {
            struct cpe_urange * curRange = ra->m_uranges + insertPos;

            assert(curRange->m_start >= start);

            if (end >= curRange->m_start) {
                curRange->m_start = start;

                if (end > curRange->m_end) {
                    curRange->m_end = end;
                }

                cpe_urange_merge_neighbers(ra, insertPos);
            }
            else if (insertPos > 0 && ra->m_uranges[insertPos - 1].m_end >= start) {
                ra->m_uranges[insertPos - 1].m_end = end;
                cpe_urange_merge_neighbers(ra, insertPos - 1);
            }
            else {
                if (cpe_urange_mgr_reserve_for_put(ra, 1) != 0) return -1;

                memmove(
                    ra->m_uranges + insertPos + 1,
                    ra->m_uranges + insertPos,
                    sizeof(struct cpe_urange) * (ra->m_urange_count - insertPos));

                curRange->m_start = start;
                curRange->m_end = end;

                ++ra->m_urange_count;
            }
        }
        else {
            if (ra->m_uranges[ra->m_urange_count - 1].m_end >= start) {
                ra->m_uranges[ra->m_urange_count - 1].m_end = end;
            }
            else {
                if (cpe_urange_mgr_reserve_for_put(ra, 1) != 0) return -1;
                assert(ra->m_uranges);
                ra->m_uranges[ra->m_urange_count].m_start = start;
                ra->m_uranges[ra->m_urange_count].m_end = end;
                ++ra->m_urange_count;
            }
        }

        if (ra->m_debug_info) {
            ptr_uint_t v = start;
            while(v != end) {
                cpe_urange_alloc_info_put(ra, v);
                ++v;
            }
        }

        return 0;
    }
}

struct cpe_urange
cpe_urange_find(cpe_urange_mgr_t ra, ptr_uint_t value) {
    int pos = cpe_urange_find_next_pos(ra, value);
    if (pos < (int)ra->m_urange_count && ra->m_uranges[pos].m_start == value) {
        return ra->m_uranges[pos];
    }

    if (pos > 0 && ra->m_uranges[pos - 1].m_end > value) {
        return ra->m_uranges[pos - 1];
    }

    return cpe_urange_invalid;
}

int cpe_urange_mgr_reserve_for_put(cpe_urange_mgr_t ra, int put_count) {
    size_t newCapacity;
    size_t requireCount;
    struct cpe_urange * newBuf;

    requireCount = ra->m_urange_count + put_count;
    newCapacity = ra->m_urange_capacity;

    while(requireCount >= newCapacity) {
        newCapacity =
            newCapacity == 0
            ? 256
            : newCapacity << 1;
    }

    if (newCapacity != ra->m_urange_capacity) {
        newBuf = (struct cpe_urange*)mem_alloc(ra->m_alloc, sizeof(struct cpe_urange) * newCapacity);
        if (newBuf == NULL) return -1;

        if (ra->m_urange_count > 0) {
            memcpy(newBuf, ra->m_uranges, sizeof(struct cpe_urange) * ra->m_urange_count);
        }

        if (ra->m_uranges) mem_free(ra->m_alloc, ra->m_uranges);

        ra->m_uranges = newBuf;
        ra->m_urange_capacity = newCapacity;
    }

    return 0;
}

int cpe_urange_is_valid(struct cpe_urange r) {
    return r.m_start <= r.m_end
        ? 1
        : 0;
}

int cpe_urange_size(struct cpe_urange r) {
    return cpe_urange_is_valid(r)
        ? (int)(r.m_end - r.m_start)
        : -1;
}

void cpe_urange_mgr_uranges(cpe_urange_it_t it, cpe_urange_mgr_t ra) {
    assert(it);
    assert(ra);

    it->m_mgr = ra;
    it->m_pos = 0;
}

struct cpe_urange cpe_urange_it_next(cpe_urange_it_t it) {
    assert(it);
    if (it->m_mgr == NULL || it->m_mgr->m_uranges == NULL) return cpe_urange_invalid;

    if (it->m_pos < (int)it->m_mgr->m_urange_count) {
        return it->m_mgr->m_uranges[it->m_pos++];
    }
    else {
        return cpe_urange_invalid;
    }
}

/*start debug support code*/
struct cpe_urange_alloc_info {
    ptr_uint_t m_value;
    uint8_t m_stack_size;
    uint8_t m_is_free;
    struct cpe_hash_entry m_hh;
};

#define CPE_RANGE_DEBUG_STACK_SIZE (10)

uint32_t cpe_urange_alloc_info_hash_fun(const struct cpe_urange_alloc_info * o) {
    return (uint32_t)o->m_value;
}

int cpe_urange_alloc_info_eq_fun(const struct cpe_urange_alloc_info * l, const struct cpe_urange_alloc_info * r) {
    return l->m_value == r->m_value;
}

void cpe_urange_alloc_info_add(cpe_urange_mgr_t ra, ptr_uint_t id) {
    struct cpe_hash_table * alloc_infos = (struct cpe_hash_table *)ra->m_debug_info;
    struct cpe_urange_alloc_info * alloc_info;
    struct cpe_urange_alloc_info key;

    key.m_value = id;
    alloc_info = cpe_hash_table_find(alloc_infos, &key);
    if (alloc_info == NULL) {
        fprintf(stderr, "urange value "FMT_PTR_INT_T" not puted!", id);
        assert(0 && "urange value not puted !!!");
        return;
    }

    if (alloc_info->m_is_free != 1) {
        struct write_stream_file ws = CPE_WRITE_STREAM_FILE_INITIALIZER(stderr, 0);
        fprintf(stderr, "urange value  "FMT_PTR_INT_T" is already allocked\n", id);
        fprintf(stderr, "   dump uranges:");
        cpe_urange_mgr_dump((write_stream_t)&ws, ra);
        fprintf(stderr, "\n");

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
        assert(0 && "urange already allocked !!!");
        return;
    }

    assert(alloc_info->m_value == id);
    alloc_info->m_is_free = 0;

#if defined CPE_HAVE_EXECINFO_H
    alloc_info->m_stack_size = backtrace((void**)(alloc_info + 1), CPE_RANGE_DEBUG_STACK_SIZE);
#endif
}

void cpe_urange_alloc_info_put(cpe_urange_mgr_t ra, ptr_uint_t id) {
    struct cpe_hash_table * alloc_infos = (struct cpe_hash_table *)ra->m_debug_info;
    struct cpe_urange_alloc_info * alloc_info;
    struct cpe_urange_alloc_info key;

    key.m_value = id;
    alloc_info = cpe_hash_table_find(alloc_infos, &key);
    if (alloc_info == NULL) {
        size_t alloc_size = sizeof(struct cpe_urange_alloc_info);

#if defined CPE_HAVE_EXECINFO_H
        alloc_size += sizeof(void*) * CPE_RANGE_DEBUG_STACK_SIZE;
#endif

        alloc_info = mem_alloc(ra->m_alloc, alloc_size);
        assert(alloc_info);
        if (alloc_info == NULL) return;

        alloc_info->m_value = id;
        alloc_info->m_is_free = 0;
        alloc_info->m_stack_size = 0;
        cpe_hash_entry_init(&alloc_info->m_hh);

        if (cpe_hash_table_insert_unique(alloc_infos, alloc_info) != 0) {
            assert(0);
        }
    }    

    if (alloc_info->m_is_free == 1) {
        struct write_stream_file ws = CPE_WRITE_STREAM_FILE_INITIALIZER(stderr, 0);
        fprintf(stderr, "urange value  "FMT_PTR_INT_T" is already put\n", id);
        fprintf(stderr, "   dump uranges:");
        cpe_urange_mgr_dump((write_stream_t)&ws, ra);
        fprintf(stderr, "\n");

#if defined CPE_HAVE_EXECINFO_H
        fprintf(stderr, "   put from\n");
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
        assert(0 && "urange value already put !!!");
        return;
    }

    assert(alloc_info->m_value == id);
    alloc_info->m_is_free = 1;

#if defined CPE_HAVE_EXECINFO_H
    alloc_info->m_stack_size = backtrace((void**)(alloc_info + 1), CPE_RANGE_DEBUG_STACK_SIZE);
#endif
}

void cpe_urange_set_debug(cpe_urange_mgr_t ra, int is_debug) {
    if (is_debug) {
        if (ra->m_debug_info) {
            return;
        }
        else {
            struct cpe_hash_table * debug_infos = mem_alloc(ra->m_alloc, sizeof(struct cpe_hash_table));
            if (debug_infos == NULL) return;

            if (cpe_hash_table_init(
                    debug_infos,
                    ra->m_alloc,
                    (cpe_hash_fun_t) cpe_urange_alloc_info_hash_fun,
                    (cpe_hash_eq_t) cpe_urange_alloc_info_eq_fun,
                    CPE_HASH_OBJ2ENTRY(cpe_urange_alloc_info, m_hh),
                    -1) != 0)
            {
                mem_free(ra->m_alloc, debug_infos);
                return;
            }
            
            ra->m_debug_info = debug_infos;
        }
    }
    else {
        if (ra->m_debug_info == NULL) {
            return;
        }
        else {
            struct cpe_hash_table * debug_infos = (struct cpe_hash_table *)ra->m_debug_info;
            struct cpe_hash_it it;
            struct cpe_urange_alloc_info * node;

            cpe_hash_it_init(&it, debug_infos);

            node = cpe_hash_it_next(&it);
            while (node) {
                struct cpe_urange_alloc_info * next = cpe_hash_it_next(&it);
                mem_free(ra->m_alloc, node);
                node = next;
            }

            cpe_hash_table_fini(debug_infos);
            mem_free(ra->m_alloc, debug_infos);
            ra->m_debug_info = NULL;
        }
    }
}

struct cpe_urange cpe_urange_invalid = { 1, 0 };
