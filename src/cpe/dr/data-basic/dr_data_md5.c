#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/dr/dr_md5.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"

struct MD5ProcessStack{
    LPDRMETA m_meta;
    LPDRMETAENTRY m_entry;
    int m_entry_pos;
    int m_entry_count;
    int m_array_pos;
    char const * m_data;
    size_t m_capacity;
};


void dr_md5_ctx_update(cpe_md5_ctx_t md5_ctx, const void * input, size_t input_capacity, LPDRMETA meta) {
    struct MD5ProcessStack processStack[CPE_DR_MAX_LEVEL];
    int stackPos;

    assert(md5_ctx);
    assert(meta);

    processStack[0].m_meta = meta;
    processStack[0].m_entry = dr_meta_entry_at(meta, 0);
    processStack[0].m_entry_pos = 0;
    processStack[0].m_entry_count = meta->m_entry_count;
    processStack[0].m_array_pos = 0;
    processStack[0].m_data = (const char *)input;
    processStack[0].m_capacity = input_capacity;

    for(stackPos = 0; stackPos >= 0;) {
        struct MD5ProcessStack * curStack;

        assert(stackPos < CPE_DR_MAX_LEVEL);

        curStack = &processStack[stackPos];
        if (curStack->m_meta == NULL) {
            --stackPos;
            continue;
        }

        for(; curStack->m_entry_pos < curStack->m_entry_count
                && curStack->m_entry;
            ++curStack->m_entry_pos
                , curStack->m_array_pos = 0
                , curStack->m_entry = dr_meta_entry_at(curStack->m_meta, curStack->m_entry_pos)
            )
        {
            size_t element_size;

        LOOPENTRY:

            element_size = dr_entry_element_size(curStack->m_entry);
            if (element_size == 0) continue;

            for(; curStack->m_array_pos < curStack->m_entry->m_array_count; ++curStack->m_array_pos) {
                char const * entry_data;
                size_t entry_capacity;
                size_t left_capacity;

                entry_data = curStack->m_data + dr_entry_data_start_pos(curStack->m_entry,  curStack->m_array_pos);
                if ((size_t)(entry_data - curStack->m_data) >= curStack->m_capacity) continue;

                left_capacity = curStack->m_capacity - (entry_data - curStack->m_data);
                entry_capacity = element_size;

                if ((curStack->m_entry_pos + 1 == curStack->m_meta->m_entry_count
                     && curStack->m_array_pos + 1 == curStack->m_entry->m_array_count) /*last element*/
                    || entry_capacity > left_capacity)
                {
                    entry_capacity = left_capacity;
                }

                if (curStack->m_entry->m_type <= CPE_DR_TYPE_COMPOSITE) {
                    struct MD5ProcessStack * nextStack;

                    if (stackPos + 1 >= CPE_DR_MAX_LEVEL) continue;

                    nextStack = &processStack[stackPos + 1];

                    nextStack->m_meta = dr_entry_ref_meta(curStack->m_entry);
                    if (nextStack->m_meta == 0) break;

                    nextStack->m_entry_pos = 0;
                    nextStack->m_entry_count = nextStack->m_meta->m_entry_count;

                    if (curStack->m_entry->m_type == CPE_DR_TYPE_UNION) {
                        LPDRMETAENTRY select_entry;
                        select_entry = dr_entry_select_entry(curStack->m_entry);
                        if (select_entry) {
                            int32_t union_entry_id;
                            dr_entry_try_read_int32(
                                &union_entry_id,
                                curStack->m_data + curStack->m_entry->m_select_data_start_pos,
                                select_entry,
                                NULL);
                                
                            nextStack->m_entry_pos =
                                dr_meta_find_entry_idx_by_id(nextStack->m_meta, union_entry_id);
                            if (nextStack->m_entry_pos < 0) continue;

                            nextStack->m_entry_count = nextStack->m_entry_pos + 1;
                        }
                    }

                    nextStack->m_entry = dr_meta_entry_at(nextStack->m_meta, nextStack->m_entry_pos);
                    nextStack->m_data = entry_data;
                    nextStack->m_capacity = entry_capacity;
                    nextStack->m_array_pos = 0;

                    ++curStack->m_array_pos;
                    ++stackPos;
                    curStack = nextStack;
                    goto LOOPENTRY;
                }
                else {
                    if (entry_capacity >= element_size) {
                        cpe_md5_ctx_update(md5_ctx, entry_data, element_size);
                    }
                }
            }
        }

        --stackPos;
    }
}

