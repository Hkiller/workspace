#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_cfg.h"
#include "cpe/dr/dr_error.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"
#include "../dr_ctype_ops.h"

int dr_cfg_read_entry(
    char * all_buf, size_t all_capacity, char * meta_buf, size_t entry_capacity,
    cfg_t cfg, LPDRMETA meta, LPDRMETAENTRY entry, int policy, error_monitor_t em);

int dr_cfg_read_struct(char * buf, size_t capacity, cfg_t cfg, LPDRMETA meta, int policy, error_monitor_t em);
int dr_cfg_read_union(char * buf, size_t capacity, cfg_t cfg, LPDRMETA meta, LPDRMETAENTRY * union_entry, int policy, error_monitor_t em);

int dr_cfg_read_entry_one(
    char * all_buf, size_t all_capacity,
    char * buf, size_t capacity,
    cfg_t cfg, LPDRMETA meta, LPDRMETAENTRY entry, int policy, error_monitor_t em)
{
    assert(entry);

    if (entry->m_type == CPE_DR_TYPE_STRUCT) {
        LPDRMETA ref;
        ref = dr_entry_ref_meta(entry);
        if (ref == NULL) {
            CPE_ERROR(
                em, "read from %s: read %s.%s, entry have no ref meta!",
                cfg_name(cfg),
                dr_meta_name(meta), cfg_name(cfg));
            return -1;
        }

        return dr_cfg_read_struct(buf, capacity, cfg, ref, policy, em);
    }
    else if (entry->m_type == CPE_DR_TYPE_UNION) {
        LPDRMETA ref;
        LPDRMETAENTRY union_entry;
        int size;

        ref = dr_entry_ref_meta(entry);
        if (ref == NULL) {
            CPE_ERROR(
                em, "read from %s: read %s.%s, entry have no ref meta!",
                cfg_name(cfg),
                dr_meta_name(meta), cfg_name(cfg));
            return -1;
        }

        union_entry = NULL;
        size = dr_cfg_read_union(buf, capacity, cfg, ref, &union_entry, policy, em);
        if (size > 0 && union_entry && union_entry->m_id != -1) {
            LPDRMETAENTRY select_entry;
            select_entry = dr_entry_select_entry(entry);
            if (select_entry) {
                size_t select_element_size;
                select_element_size = dr_entry_element_size(select_entry);
                if (entry->m_select_data_start_pos + select_element_size <= all_capacity) {
                    dr_entry_set_from_int32(
                        all_buf + entry->m_select_data_start_pos,
                        union_entry->m_id,
                        select_entry, em);
                }
            }
        }

        return size;
    }
    else {
        dr_entry_set_from_ctype(
            buf,
            cfg_data(cfg),
            cfg_type(cfg),
            entry,
            em);
        return entry->m_unitsize;
    }
}

void dr_cfg_read_array_set_dft(size_t count, size_t max_count, size_t element_size, char * meta_buf, LPDRMETA meta, LPDRMETAENTRY entry) {
    const void * dftValue;

    dftValue = dr_entry_dft_value(entry);
    if (dftValue) {
        while(count < max_count) {
            memcpy(meta_buf + dr_entry_data_start_pos(entry, (int)count), dftValue, element_size);
            ++count;
        }
    }
    else {
        while(count < max_count) {
            bzero(meta_buf + dr_entry_data_start_pos(entry, (int)count), element_size);
            ++count;
        }
    }
}

int dr_cfg_read_entry(
    char * all_buf, size_t all_capacity,
    char * meta_buf, size_t entry_capacity,
    cfg_t cfg, LPDRMETA meta, LPDRMETAENTRY entry, int policy, error_monitor_t em)
{
    cfg_it_t itemIt;
    cfg_t item;

    assert(entry);

    if (entry->m_array_count == 1) {
        return dr_cfg_read_entry_one(
            all_buf, all_capacity,
            meta_buf + dr_entry_data_start_pos(entry, 0), entry_capacity,
            cfg, meta, entry, policy, em);
    }
    else {
        LPDRMETAENTRY refer;
        int count;
        size_t max_count;
        size_t element_size = dr_entry_element_size(entry);
        if (element_size == 0) {
            CPE_ERROR(
                em, "read from %s: read %s.%s, element size is unknown!",
                cfg_name(cfg),
                dr_meta_name(meta), cfg_name(cfg));
            return entry->m_unitsize;
        }

        count = 0;
        max_count = entry->m_array_count;

        if (max_count == 0) {
            max_count = dr_entry_array_calc_ele_num(entry, entry_capacity);
        }
        else {
            if (dr_entry_array_calc_buf_capacity(entry, max_count) > entry_capacity) {
                CPE_ERROR(
                    em,
                    "process %s.%s, array element overflow, require " FMT_SIZE_T ", capacity is " FMT_SIZE_T "!",
                    dr_meta_name(meta), dr_entry_name(entry),
                    max_count * element_size, entry_capacity);
                max_count = dr_entry_array_calc_ele_num(entry, entry_capacity);
            }
        }

        cfg_it_init(&itemIt, cfg);
        while((item = cfg_it_next(&itemIt)) && count < max_count) {
            dr_cfg_read_entry_one(
                all_buf, all_capacity,
                meta_buf + dr_entry_data_start_pos(entry, count),
                element_size, item, meta, entry, policy, em);
            ++count;
        }

        refer = dr_entry_array_refer_entry(entry);
        if (refer) {
            size_t refer_element_size = dr_entry_element_size(refer);
            if (entry->m_array_refer_data_start_pos + refer_element_size < all_capacity) {
                dr_entry_set_from_int32(
                    all_buf + entry->m_array_refer_data_start_pos,
                    count,
                    refer,
                    em);
            }
        }
        else {
            dr_cfg_read_array_set_dft(count, max_count, element_size, meta_buf, meta, entry);
            count = (int)max_count;
        }

        return (int)(count * element_size);
    }
}

int dr_cfg_read_union(char * buf, size_t capacity, cfg_t cfg, LPDRMETA meta, LPDRMETAENTRY * union_entry, int policy, error_monitor_t em) {
    cfg_it_t itemIt;
    cfg_t item;
    size_t size;

    size = dr_meta_size(meta);

    cfg_it_init(&itemIt, cfg);

    for(item = cfg_it_next(&itemIt); item; item = cfg_it_next(&itemIt)) {
        int entry_size;
        size_t entry_capacity;
        LPDRMETAENTRY entry;

        entry = dr_meta_find_entry_by_name(meta, cfg_name(item));
        if (entry == NULL) {
            if (policy | DR_CFG_READ_CHECK_NOT_EXIST_ATTR) {
                CPE_WARNING(
                    em, "read from %s: %s have no entry %s, ignore!",
                    cfg_name(cfg), dr_meta_name(meta), cfg_name(item));
            }
            continue;
        }

        if ((size_t)entry->m_data_start_pos >= capacity) continue;

        entry_capacity = capacity - entry->m_data_start_pos;

        entry_size = dr_cfg_read_entry(buf, capacity, buf, entry_capacity, item, meta, entry, policy, em);
        if (entry_size < 0) continue;

        if (union_entry) *union_entry = entry;

        if (entry->m_data_start_pos + entry_size > size) {
            size = (size_t)(entry->m_data_start_pos + entry_size);
        }
    }
    
    return (int)size;
}

int dr_cfg_read_struct(char * buf, size_t capacity, cfg_t cfg, LPDRMETA meta, int policy, error_monitor_t em) {
    cfg_it_t itemIt;
    cfg_t item;
    size_t size;
    LPDRMETAENTRY last_entry;
    
    assert(cfg);

    size = dr_meta_size(meta);

    cfg_it_init(&itemIt, cfg);

    last_entry = 0;
    if (dr_meta_entry_num(meta) > 0) {
        last_entry = dr_meta_entry_at(meta, dr_meta_entry_num(meta) - 1);
    }

    for(item = cfg_it_next(&itemIt); item; item = cfg_it_next(&itemIt)) {
        size_t entry_size;
        size_t entry_capacity;
        LPDRMETAENTRY entry;

        entry = dr_meta_find_entry_by_name(meta, cfg_name(item));
        if (entry == NULL) {
            if (policy | DR_CFG_READ_CHECK_NOT_EXIST_ATTR) {
                CPE_WARNING(
                    em, "read from %s: %s have no entry %s, ignore!",
                    cfg_name(cfg), dr_meta_name(meta), cfg_name(item));
            }
            continue;
        }

        if ((size_t)entry->m_data_start_pos >= capacity) continue;

        entry_capacity = entry->m_unitsize;
        if (entry == last_entry || entry->m_data_start_pos + entry_capacity > capacity) {
            entry_capacity = capacity - entry->m_data_start_pos;
        }

        entry_size = dr_cfg_read_entry(buf, capacity, buf, entry_capacity, item, meta, entry, policy, em);
        
        if ((int)(entry->m_data_start_pos + entry_size) > size) {
            size = (size_t)(entry->m_data_start_pos + entry_size);
        }
    }
    
    return (int)size;
}

int dr_cfg_read_i(
    char * buf,
    size_t capacity,
    cfg_t cfg,
    LPDRMETA meta,
    int policy,
    error_monitor_t em)
{
    if (meta == NULL) {
        CPE_ERROR(em, "read from %s: meta is empty!", cfg_name(cfg));
        return 0;
    }

    if (meta->m_type == CPE_DR_TYPE_STRUCT) {
        return dr_cfg_read_struct(buf, capacity, cfg, meta, policy, em);
    }
    else {
        assert(meta->m_type == CPE_DR_TYPE_UNION);
        return dr_cfg_read_union(buf, capacity, cfg, meta, NULL, policy, em);
    }
}

int dr_cfg_read(
    void * buf,
    size_t capacity,
    cfg_t cfg,
    LPDRMETA meta,
    int policy,
    error_monitor_t em)
{
    int ret = 0;
    int size = 0;

    if (cfg == NULL) {
        CPE_ERROR(em, "dr_cfg_read: input cfg is null!");
        return -1;
    }

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        size = dr_cfg_read_i((char *)buf, capacity, cfg, meta, policy, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        size = dr_cfg_read_i((char *)buf, capacity, cfg, meta, policy, &logError);
    }

    return ret ? ret : size;
}
