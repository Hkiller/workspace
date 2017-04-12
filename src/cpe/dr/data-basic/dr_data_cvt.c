#include <assert.h>
#include <string.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"

struct DataCvtProcessStack{
    cfg_t m_cvt_cfg;

    LPDRMETA m_des_meta;
    LPDRMETAENTRY m_des_entry;
    int32_t m_des_entry_pos;
    int32_t m_des_entry_count;
    char * m_des_data;
    size_t m_des_capacity;

    LPDRMETA m_src_meta;
    char const * m_src_data;
    size_t m_src_capacity;

    int m_array_pos;

    int m_sub_pos;
};

int dr_meta_data_cvt_check_type_copyable(int desType, int srcType) {
    if (desType <= CPE_DR_TYPE_COMPOSITE) {
        return srcType == desType ? 1 : 0;
    }
    else {
        return srcType > CPE_DR_TYPE_COMPOSITE ? 1 : 0;
    }
}

int dr_meta_data_cvt_check_array_copyable(int desArrayCount, int srcArrayCount) {
    return (desArrayCount == 1 && srcArrayCount == 1)
        || (desArrayCount != 1 && srcArrayCount != 1)
        ? 1
        : 0;
}

const char *
dr_meta_data_cvt_union_find_select_entry_name(
    const char * src_data, size_t src_capacity, LPDRMETAENTRY src_entry, LPDRMETA src_meta) 
{
    LPDRMETAENTRY src_select_entry;
    LPDRMETAENTRY src_union_entry;
    int32_t src_union_entry_id;
    int32_t src_union_entry_pos;

    src_select_entry = dr_entry_select_entry(src_entry);
    if (src_select_entry == 0) return NULL;

    if (src_entry->m_select_data_start_pos + dr_entry_element_size(src_select_entry) > src_capacity) 
        return NULL;

    if (dr_entry_try_read_int32(
            &src_union_entry_id,
            src_data + src_entry->m_select_data_start_pos,
            src_select_entry,
            NULL) != 0)
    {
        return NULL;
    }

    src_union_entry_pos = dr_meta_find_entry_idx_by_id(src_meta, src_union_entry_id);
    if (src_union_entry_pos < 0) return NULL;

    src_union_entry = dr_meta_entry_at(src_meta, src_union_entry_pos);
    if (src_union_entry == NULL) return NULL;

    return dr_entry_name(src_union_entry);
}

const char * dr_data_cvt_src_name_to_des_name(cfg_t cfg, const char * name) {
    struct cfg_it it;
    cfg_t child_cfg;

    cfg_it_init(&it, cfg);
    while((child_cfg = cfg_it_next(&it))) {
        if (cfg_type(child_cfg) == CPE_CFG_TYPE_STRING) {
            if (strstr((const char *)cfg_data(child_cfg), name) == 0) {
                return cfg_name(child_cfg);
            }
        }
        else if (cfg_type(child_cfg) == CPE_CFG_TYPE_STRUCT) {
            cfg_t only_child = cfg_child_only(child_cfg);
            if (only_child && strcmp(cfg_name(only_child), name) == 0) {
                return cfg_name(child_cfg);
            }
        }
    }

    return NULL;
}

void dr_data_cvt(
    void * des_data, size_t des_capacity, LPDRMETA des_meta,
    void const * src_data, size_t src_capacity, LPDRMETA src_meta, 
    cfg_t cfg, error_monitor_t em)
{
    struct DataCvtProcessStack processStack[CPE_DR_MAX_LEVEL];
    int stackPos;

    assert(cfg);
    assert(des_data);
    assert(des_meta);
    assert(src_data);
    assert(src_meta);

    processStack[0].m_cvt_cfg = cfg;
    processStack[0].m_des_meta = des_meta;
    processStack[0].m_des_entry = dr_meta_entry_at(des_meta, 0);
    processStack[0].m_des_entry_pos = 0;
    processStack[0].m_des_entry_count = des_meta->m_entry_count;
    processStack[0].m_array_pos = 0;
    processStack[0].m_des_data = (char *)des_data;
    processStack[0].m_des_capacity = des_capacity;
    processStack[0].m_src_meta = src_meta;
    processStack[0].m_src_data = (const char *)src_data;
    processStack[0].m_src_capacity = src_capacity;
    processStack[0].m_sub_pos = 0;

    for(stackPos = 0; stackPos >= 0;) {
        struct DataCvtProcessStack * curStack;

        assert(stackPos < CPE_DR_MAX_LEVEL);

        curStack = &processStack[stackPos];
        if (curStack->m_des_meta == NULL) {
            --stackPos;
            continue;
        }

        for(; curStack->m_des_entry_pos < curStack->m_des_entry_count
                && curStack->m_des_entry
                ; ++curStack->m_des_entry_pos
                , curStack->m_array_pos = 0
                , curStack->m_des_entry = dr_meta_entry_at(curStack->m_des_meta, curStack->m_des_entry_pos)
            )
        {
            size_t des_element_size, src_element_size;
            const char * src_entry_name;
            LPDRMETAENTRY src_entry;
            int32_t des_array_count_max, src_array_count_max;
            cfg_t entry_all_cfg;
            cfg_t entry_cur_cfg;
            
        LOOPENTRY:
            des_element_size = dr_entry_element_size(curStack->m_des_entry);
            if (des_element_size == 0) continue;

            entry_all_cfg = cfg_find_cfg(curStack->m_cvt_cfg, dr_entry_name(curStack->m_des_entry));
            if (entry_all_cfg == NULL) continue;

            if (cfg_type(entry_all_cfg) == CPE_CFG_TYPE_STRUCT) {
                struct cfg_it it;
                cfg_it_init(&it, entry_all_cfg);

                entry_cur_cfg = cfg_it_next(&it);

                if (entry_cur_cfg == NULL) continue;
                src_entry_name = cfg_name(entry_cur_cfg);
            }
            else if (cfg_type(entry_all_cfg) == CPE_CFG_TYPE_STRING) {
                src_entry_name = (const char *)cfg_data(entry_all_cfg);
                entry_cur_cfg = NULL;
            }
            else {
                continue;
            }

            src_entry = dr_meta_find_entry_by_name(curStack->m_src_meta, src_entry_name);
            if (src_entry == 0) continue;

            src_element_size = dr_entry_element_size(src_entry);
            if (src_element_size == 0) continue;

            if (!dr_meta_data_cvt_check_type_copyable(curStack->m_des_entry->m_type, src_entry->m_type)) continue;

            if (!dr_meta_data_cvt_check_array_copyable(curStack->m_des_entry->m_array_count, src_entry->m_array_count)) continue;

            des_array_count_max = curStack->m_des_entry->m_array_count;
            src_array_count_max = src_entry->m_array_count;
            if (src_array_count_max != 1) {
                LPDRMETAENTRY srcRefer = dr_entry_array_refer_entry(src_entry);
                if (srcRefer) {
                    int32_t readBuf;
                    if (dr_ctype_try_read_int32(
                            &readBuf, 
                            curStack->m_src_data + src_entry->m_array_refer_data_start_pos,
                            srcRefer->m_type,
                            0) == 0)
                    {
                        src_array_count_max = readBuf;
                    }
                }
            }

            for(; curStack->m_array_pos < des_array_count_max && curStack->m_array_pos < src_array_count_max
                    ; ++curStack->m_array_pos)
            {
                char * des_entry_data;
                const char * src_entry_data;
                size_t des_entry_capacity, des_left_capacity;
                size_t src_entry_capacity, src_left_capacity;

                des_entry_data = curStack->m_des_data + dr_entry_data_start_pos(curStack->m_des_entry, curStack->m_array_pos);
                if ((size_t)(des_entry_data - curStack->m_des_data) > curStack->m_des_capacity) continue;

                des_left_capacity = curStack->m_des_capacity - (des_entry_data - curStack->m_des_data);
                des_entry_capacity = des_element_size;

                if ((curStack->m_des_entry_pos + 1 == curStack->m_des_meta->m_entry_count
                     && curStack->m_array_pos + 1 == des_array_count_max) /*last element*/
                    || des_entry_capacity > des_left_capacity)
                {
                    des_entry_capacity = des_left_capacity;
                }

                src_entry_data = curStack->m_src_data + dr_entry_data_start_pos(src_entry, curStack->m_array_pos);
                if ((size_t)(src_entry_data - curStack->m_src_data) > curStack->m_src_capacity) continue;

                src_left_capacity = curStack->m_src_capacity - (src_entry_data - curStack->m_src_data);
                src_entry_capacity = src_element_size;
                
                if ((src_entry == dr_meta_entry_at(curStack->m_src_meta, curStack->m_src_meta->m_entry_count - 1)
                     && curStack->m_array_pos + 1 == src_array_count_max) /*last element*/
                    || des_entry_capacity > des_left_capacity)
                {
                    src_entry_capacity = src_left_capacity;
                }

                if (curStack->m_des_entry->m_type <= CPE_DR_TYPE_COMPOSITE) {
                    struct DataCvtProcessStack * nextStack;

                    if (stackPos + 1 >= CPE_DR_MAX_LEVEL)  continue;

                    nextStack = &processStack[stackPos + 1];

                    nextStack->m_des_meta = dr_entry_ref_meta(curStack->m_des_entry);
                    if (nextStack->m_des_meta == 0) continue;

                    nextStack->m_src_meta = dr_entry_ref_meta(src_entry);
                    if (nextStack->m_src_meta == 0) continue;

                    nextStack->m_cvt_cfg = entry_cur_cfg;
                    nextStack->m_sub_pos = 0;

                    nextStack->m_des_data = des_entry_data;
                    nextStack->m_des_capacity = des_entry_capacity;
                    nextStack->m_array_pos = 0;

                    nextStack->m_src_data = src_entry_data;
                    nextStack->m_src_capacity = src_entry_capacity;

                    nextStack->m_des_entry_pos = 0;
                    nextStack->m_des_entry_count = nextStack->m_des_meta->m_entry_count;
                    nextStack->m_des_entry = dr_meta_entry_at(nextStack->m_des_meta, 0);

                    if (curStack->m_des_entry->m_type == CPE_DR_TYPE_UNION) {
                        const char * src_union_entry_name;
                        const char * des_union_entry_name;

                        src_union_entry_name =
                            dr_meta_data_cvt_union_find_select_entry_name(
                                curStack->m_src_data,
                                curStack->m_src_capacity,
                                src_entry,
                                nextStack->m_src_meta);

                        des_union_entry_name =
                            src_union_entry_name
                            ? dr_data_cvt_src_name_to_des_name(entry_cur_cfg, src_union_entry_name)
                            : NULL;

                        if (des_union_entry_name) {
                            nextStack->m_des_entry_pos = dr_meta_find_entry_idx_by_name(nextStack->m_des_meta, des_union_entry_name);
                            if (nextStack->m_des_entry_pos < 0) continue;

                            nextStack->m_des_entry = dr_meta_entry_at(nextStack->m_des_meta, nextStack->m_des_entry_pos);
                            nextStack->m_des_entry_count = nextStack->m_des_entry_pos + 1;

                            if (nextStack->m_des_entry->m_id != -1) {
                                LPDRMETAENTRY des_select_entry;
                                des_select_entry = dr_entry_select_entry(curStack->m_des_entry);
                                if (des_select_entry) {
                                    if (curStack->m_des_entry->m_select_data_start_pos + dr_entry_element_size(des_select_entry)
                                        <= curStack->m_des_capacity)
                                    {
                                        dr_entry_set_from_int32(
                                            curStack->m_des_data + curStack->m_des_entry->m_select_data_start_pos,
                                            nextStack->m_des_entry->m_id,
                                            des_select_entry, NULL);
                                    }
                                }
                            }
                        }
                    }

                    if (nextStack->m_des_entry == 0) {
                    }

                    ++curStack->m_array_pos;
                    ++stackPos;
                    curStack = nextStack;
                    goto LOOPENTRY;
                }
                else {
                    dr_entry_set_from_ctype(des_entry_data, src_entry_data, src_entry->m_type, curStack->m_des_entry, 0);
                }
            }

            if (curStack->m_des_entry->m_array_count != 1) {
                LPDRMETAENTRY refer = dr_entry_array_refer_entry(curStack->m_des_entry);
                if (refer) {
                    dr_entry_set_from_int32(
                        curStack->m_des_data + curStack->m_des_entry->m_array_refer_data_start_pos,
                        curStack->m_array_pos,
                        refer,
                        0);
                }
            }
        }

        --stackPos;
    }
}
