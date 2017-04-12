#include <assert.h>
#include "yajl/yajl_parse.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_json_tree.h"
#include "cpe/dr/dr_error.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"
#include "../dr_ctype_ops.h"

int dr_json_tree_read_entry(
    char * all_buf, size_t all_capacity, char * meta_buf, size_t entry_capacity,
    const char * val_name, yajl_val val, LPDRMETA meta, LPDRMETAENTRY entry, error_monitor_t em);

int dr_json_tree_read_struct(char * buf, size_t capacity, const char * val_name, yajl_val val, LPDRMETA meta, error_monitor_t em);
int dr_json_tree_read_union(char * buf, size_t capacity, const char * val_name, yajl_val val, LPDRMETA meta, LPDRMETAENTRY * union_entry, error_monitor_t em);

int dr_json_tree_read_entry_one(
    char * all_buf, size_t all_capacity,
    char * buf, size_t capacity,
    const char * val_name, yajl_val val, LPDRMETA meta, LPDRMETAENTRY entry, error_monitor_t em)
{
    assert(entry);

    if (entry->m_type == CPE_DR_TYPE_STRUCT) {
        LPDRMETA ref;
        ref = dr_entry_ref_meta(entry);
        if (ref == NULL) {
            CPE_ERROR(
                em, "read from %s: read %s.%s, entry have no ref meta!",
                val_name,
                dr_meta_name(meta), val_name);
            return -1;
        }

        return dr_json_tree_read_struct(buf, capacity, val_name, val, ref, em);
    }
    else if (entry->m_type == CPE_DR_TYPE_UNION) {
        LPDRMETA ref;
        LPDRMETAENTRY union_entry;
        int size;

        ref = dr_entry_ref_meta(entry);
        if (ref == NULL) {
            CPE_ERROR(
                em, "read from %s: read %s.%s, entry have no ref meta!",
                val_name,
                dr_meta_name(meta), val_name);
            return -1;
        }

        union_entry = NULL;
        size = dr_json_tree_read_union(buf, capacity, val_name, val, ref, &union_entry, em);
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
        switch(val->type) {
        case yajl_t_string:
            dr_entry_set_from_string(buf, val->u.string, entry, em);
            break;
        case yajl_t_number:
            if (val->u.number.flags & YAJL_NUMBER_INT_VALID) {
                dr_entry_set_from_int64(buf, (int64_t)val->u.number.i, entry, em);
            }
            if (val->u.number.flags & YAJL_NUMBER_DOUBLE_VALID) {
                dr_entry_set_from_double(buf, val->u.number.d, entry, em);
            }
            else {
                dr_entry_set_from_string(buf, val->u.number.r, entry, em);
            }
            break;
        case yajl_t_true:
            dr_entry_set_from_uint8(buf, 1, entry, em);
            break;
        case yajl_t_false:
            dr_entry_set_from_uint8(buf, 0, entry, em);
            break;
        case yajl_t_null:
            CPE_ERROR(em, "read from %s: read %s.%s, not support set type null!", val_name, dr_meta_name(meta), val_name);
            break;
        case yajl_t_object:
            CPE_ERROR(em, "read from %s: read %s.%s, not support set type object!", val_name, dr_meta_name(meta), val_name);
            break;
        case yajl_t_array:
            CPE_ERROR(em, "read from %s: read %s.%s, not support set type array!", val_name, dr_meta_name(meta), val_name);
            break;
        default:
            CPE_ERROR(em, "read from %s: read %s.%s, not support set type unknown!", val_name, dr_meta_name(meta), val_name);
            break;
        }

        return entry->m_unitsize;
    }
}

void dr_json_tree_read_array_set_dft(size_t count, size_t max_count, size_t element_size, char * meta_buf, LPDRMETA meta, LPDRMETAENTRY entry) {
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

int dr_json_tree_read_entry(
    char * all_buf, size_t all_capacity,
    char * meta_buf, size_t entry_capacity,
    const char * val_name, yajl_val val, LPDRMETA meta, LPDRMETAENTRY entry, error_monitor_t em)
{
    size_t i;

    assert(entry);

    if (entry->m_array_count == 1) {
        return dr_json_tree_read_entry_one(
            all_buf, all_capacity,
            meta_buf + dr_entry_data_start_pos(entry, 0), entry_capacity,
            val_name, val, meta, entry, em);
    }
    else {
        LPDRMETAENTRY refer;
        int count;
        size_t max_count;
        size_t element_size;

        if (val->type != yajl_t_array) {
            CPE_ERROR(
                em, "read from %s: read %s.%s, input value is not array!",
                val_name,
                dr_meta_name(meta), val_name);
            return entry->m_unitsize;
        }
        
        element_size = dr_entry_element_size(entry);
        if (element_size == 0) {
            CPE_ERROR(
                em, "read from %s: read %s.%s, element size is unknown!",
                val_name,
                dr_meta_name(meta), val_name);
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

        for(i = 0; i < val->u.array.len && count < max_count; ++i) {
            char valu_name_buf[64];
            snprintf(valu_name_buf, sizeof(valu_name_buf), "%s[%d]", val_name, (int)i);
            
            dr_json_tree_read_entry_one(
                all_buf, all_capacity,
                meta_buf + dr_entry_data_start_pos(entry, count),
                element_size, val_name, val->u.array.values[i], meta, entry, em);
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
            dr_json_tree_read_array_set_dft(count, max_count, element_size, meta_buf, meta, entry);
            count = (int)max_count;
        }

        return (int)(count * element_size);
    }
}

int dr_json_tree_read_union(char * buf, size_t capacity, const char * val_name, yajl_val val, LPDRMETA meta, LPDRMETAENTRY * union_entry, error_monitor_t em) {
    size_t size;
    size_t i;
    
    if (val->type != yajl_t_object) {
        CPE_ERROR(em, "read from %s: val is not struct!", val_name);
        return 0;
    }
    
    size = dr_meta_size(meta);

    for(i = 0; i < val->u.object.len; i++) {
        yajl_val item = val->u.object.values[i];
        const char * item_name = val->u.object.keys[i];
        int entry_size;
        size_t entry_capacity;
        LPDRMETAENTRY entry;

        entry = dr_meta_find_entry_by_name(meta, item_name);
        if (entry == NULL) continue;

        if ((size_t)entry->m_data_start_pos >= capacity) continue;

        entry_capacity = capacity - entry->m_data_start_pos;

        entry_size = dr_json_tree_read_entry(buf, capacity, buf, entry_capacity, item_name, item, meta, entry, em);
        if (entry_size < 0) continue;

        if (union_entry) *union_entry = entry;

        if (entry->m_data_start_pos + entry_size > size) {
            size = (size_t)(entry->m_data_start_pos + entry_size);
        }
    }
    
    return (int)size;
}

int dr_json_tree_read_struct(char * buf, size_t capacity, const char * val_name, yajl_val val, LPDRMETA meta, error_monitor_t em) {
    size_t size;
    size_t i;
    LPDRMETAENTRY last_entry;
    
    assert(val);

    if (val->type != yajl_t_object) {
        CPE_ERROR(em, "read from %s: val is not struct!", val_name);
        return 0;
    }
    
    size = dr_meta_size(meta);

    last_entry = 0;
    if (dr_meta_entry_num(meta) > 0) {
        last_entry = dr_meta_entry_at(meta, dr_meta_entry_num(meta) - 1);
    }

    for(i = 0; i < val->u.object.len; ++i) {
        yajl_val item = val->u.object.values[i];
        const char * item_name = val->u.object.keys[i];
        size_t entry_size;
        size_t entry_capacity;
        LPDRMETAENTRY entry;

        entry = dr_meta_find_entry_by_name(meta, item_name);
        if (entry == NULL) continue;

        if ((size_t)entry->m_data_start_pos >= capacity) continue;

        entry_capacity = entry->m_unitsize;
        if (entry == last_entry || entry->m_data_start_pos + entry_capacity > capacity) {
            entry_capacity = capacity - entry->m_data_start_pos;
        }

        entry_size = dr_json_tree_read_entry(buf, capacity, buf, entry_capacity, item_name, item, meta, entry, em);
        
        if ((int)(entry->m_data_start_pos + entry_size) > size) {
            size = (size_t)(entry->m_data_start_pos + entry_size);
        }
    }
    
    return (int)size;
}

int dr_json_tree_read(void * result, size_t capacity, yajl_val input, LPDRMETA meta, error_monitor_t em) {
    int ret = 0;
    int size = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        size = dr_json_tree_read_struct(result, capacity, "root", input, meta, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        size = dr_json_tree_read_struct(result, capacity, "root", input, meta, &logError);
    }

    return ret == 0 ? size : ret;
}
