#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/dr/dr_cfg.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"
#include "../dr_ctype_ops.h"

struct SetDefaultProcessStack{
    LPDRMETA m_meta;
    LPDRMETAENTRY m_entry;
    int m_entry_pos;
    int m_entry_count;
    int m_array_pos;
    cfg_t m_des;
    cfg_t m_des_seq;
    const char * m_src_data;
};

void dr_cfg_write_i(
    cfg_t cfg,
    const void * data,
    LPDRMETA meta,
    error_monitor_t em)
{
    struct SetDefaultProcessStack processStack[CPE_DR_MAX_LEVEL];
    int stackPos;

    assert(cfg);
    assert(meta);
    assert(data);

    processStack[0].m_meta = meta;
    processStack[0].m_entry = dr_meta_entry_at(meta, 0);
    processStack[0].m_entry_pos = 0;
    processStack[0].m_entry_count = meta->m_entry_count;
    processStack[0].m_array_pos = 0;
    processStack[0].m_des = cfg;
    processStack[0].m_des_seq = NULL;
    processStack[0].m_src_data = (const char *)data;

    for(stackPos = 0; stackPos >= 0;) {
        struct SetDefaultProcessStack * curStack;

        assert(stackPos < CPE_DR_MAX_LEVEL);

        curStack = &processStack[stackPos];
        if (curStack->m_meta == NULL) {
            --stackPos;
            continue;
        }

        for(; curStack->m_entry_pos < curStack->m_entry_count
                && curStack->m_entry
                && curStack->m_des;
            ++curStack->m_entry_pos
                , curStack->m_array_pos = 0
                , curStack->m_entry = dr_meta_entry_at(curStack->m_meta, curStack->m_entry_pos)
                , curStack->m_des_seq = NULL
            )
        {
            size_t elementSize;
            int32_t array_count;
            LPDRMETAENTRY refer;

        LOOPENTRY:

            elementSize = dr_entry_element_size(curStack->m_entry);
            if (elementSize == 0) continue;

            refer = NULL;
            if (curStack->m_entry->m_array_count != 1) {
                curStack->m_des_seq =
                    cfg_struct_add_seq(curStack->m_des, dr_entry_name(curStack->m_entry), cfg_merge_use_exist);

                refer = dr_entry_array_refer_entry(curStack->m_entry);
            }

            array_count = curStack->m_entry->m_array_count;
            if (refer) {
                dr_entry_try_read_int32(
                    &array_count,
                    curStack->m_src_data + curStack->m_entry->m_array_refer_data_start_pos,
                    refer,
                    em);
            }

            for(; curStack->m_array_pos < array_count; ++curStack->m_array_pos) {
                const char * entryData = curStack->m_src_data + dr_entry_data_start_pos(curStack->m_entry, curStack->m_array_pos);

                if (curStack->m_entry->m_type <= CPE_DR_TYPE_COMPOSITE) {
                    if (stackPos + 1 < CPE_DR_MAX_LEVEL) {
                        struct SetDefaultProcessStack * nextStack;
                        nextStack = &processStack[stackPos + 1];

                        nextStack->m_meta = dr_entry_ref_meta(curStack->m_entry);
                        if (nextStack->m_meta == 0) break;

                        nextStack->m_src_data = entryData;
                        nextStack->m_entry_pos = 0;
                        nextStack->m_entry_count = nextStack->m_meta->m_entry_count;

                        if (curStack->m_entry->m_type == CPE_DR_TYPE_UNION) {
                            LPDRMETAENTRY select_entry;
                            select_entry = dr_entry_select_entry(curStack->m_entry);
                            if (select_entry) {
                                int32_t union_entry_id;
                                dr_entry_try_read_int32(
                                    &union_entry_id,
                                    curStack->m_src_data + curStack->m_entry->m_select_data_start_pos,
                                    select_entry,
                                    em);
                                
                                nextStack->m_entry_pos =
                                    dr_meta_find_entry_idx_by_id(nextStack->m_meta, union_entry_id);
                                if (nextStack->m_entry_pos < 0) continue;

                                nextStack->m_entry_count = nextStack->m_entry_pos + 1;
                            }
                        }

                        nextStack->m_entry = dr_meta_entry_at(nextStack->m_meta, nextStack->m_entry_pos);

                        nextStack->m_array_pos = 0;
                        nextStack->m_des =
                            curStack->m_des_seq
                            ? cfg_seq_add_struct(curStack->m_des_seq)
                            : cfg_struct_add_struct(curStack->m_des, dr_entry_name(curStack->m_entry), cfg_merge_use_exist);
                        nextStack->m_des_seq = NULL;

                        ++curStack->m_array_pos;
                        ++stackPos;
                        curStack = nextStack;
                        goto LOOPENTRY;
                    }
                }
                else {
                    if (curStack->m_des_seq) {
                        cfg_seq_add_value_from_binary(
                            curStack->m_des_seq, curStack->m_entry->m_type, entryData);
                    }
                    else {
                        cfg_struct_add_value_from_binary(
                            curStack->m_des, dr_entry_name(curStack->m_entry), curStack->m_entry->m_type , entryData, cfg_merge_use_new);
                    }
                }
            }
        }

        --stackPos;
    }
}

int dr_cfg_write(
    cfg_t cfg,
    const void * data,
    LPDRMETA meta,
    error_monitor_t em)
{
    int ret = 0;

    if (cfg == NULL) {
        CPE_ERROR(em, "dr_cfg_read: input cfg is null!");
        return -1;
    }

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        dr_cfg_write_i(cfg, data, meta, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        dr_cfg_write_i(cfg, data, meta, &logError);
    }

    return ret;
}
