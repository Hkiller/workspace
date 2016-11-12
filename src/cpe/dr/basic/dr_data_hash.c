#include "cpe/pal/pal_string.h"
#include "cpe/utils/hash.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"

uint32_t dr_ctype_hash(const void * data, int type) {
    switch(type) {
    case CPE_DR_TYPE_CHAR:
    case CPE_DR_TYPE_UCHAR:
    case CPE_DR_TYPE_INT8:
    case CPE_DR_TYPE_UINT8:
        return *(uint8_t *)data;
    case CPE_DR_TYPE_INT16:
    case CPE_DR_TYPE_UINT16:
        return *(uint16_t *)data;
    case CPE_DR_TYPE_INT32:
    case CPE_DR_TYPE_UINT32:
        return *(uint32_t *)data;
    case CPE_DR_TYPE_INT64:
    case CPE_DR_TYPE_UINT64:
        return (*(uint64_t *)data) & 0xFFFFFFFF;
    case CPE_DR_TYPE_FLOAT:
    case CPE_DR_TYPE_DOUBLE:
        return (*(uint64_t *)data) & 0xFFFFFFFF;
    case CPE_DR_TYPE_STRING:
        return cpe_hash_str((const char *)data, strlen((const char *)data));
    case CPE_DR_TYPE_DATE:
    case CPE_DR_TYPE_TIME:
    case CPE_DR_TYPE_DATETIME:
    case CPE_DR_TYPE_MONEY:
    case CPE_DR_TYPE_IP:
    case CPE_DR_TYPE_VOID:
    default:
        return type;
    }
}

uint32_t dr_entry_hash(const void * input, LPDRMETAENTRY entry) {
    const char * data;

    data = ((const char *)input) + entry->m_data_start_pos;

    switch(entry->m_type) {
    case CPE_DR_TYPE_UNION: {
        const char * name;
        LPDRMETAENTRY select_entry;

        select_entry = dr_entry_select_entry(entry);
        if (select_entry) {
            int32_t union_entry_id;
            int32_t use_entry_pos;
            dr_entry_try_read_int32(
                &union_entry_id,
                ((const char *)input) + entry->m_select_data_start_pos,
                select_entry,
                NULL);
                                
            use_entry_pos = dr_meta_find_entry_idx_by_id(dr_entry_self_meta(entry), union_entry_id);
            if (use_entry_pos >= 0) {
                return dr_entry_hash(data, dr_meta_entry_at(dr_entry_self_meta(entry), use_entry_pos));
            }

        }

        name = dr_entry_name(entry);
        return cpe_hash_str(name, strlen(name));
    }
    case CPE_DR_TYPE_STRUCT: {
        const char * name;
        LPDRMETA meta;
        size_t len;
        size_t i;
        uint32_t h;
        size_t step;

        name = dr_entry_name(entry);

        meta = dr_entry_ref_meta(entry);
        if (meta == NULL) return cpe_hash_str(name, strlen(name));

        len = dr_meta_entry_num(meta);
        h = (uint32_t)len;
        step = (h >> 5) + 1;
        
        for (i = len; i >= step; i -= step) {
            uint32_t nh = dr_entry_hash(data, dr_meta_entry_at(meta, (int)(i - 1)));
            h = h ^ ((h<<5)+(h>>2) + nh);
        }

        return h;
    }
    case CPE_DR_TYPE_CHAR:
    case CPE_DR_TYPE_UCHAR:
    case CPE_DR_TYPE_INT8:
    case CPE_DR_TYPE_UINT8:
        return *(uint8_t *)data;
    case CPE_DR_TYPE_INT16:
    case CPE_DR_TYPE_UINT16:
        return *(uint16_t *)data;
    case CPE_DR_TYPE_INT32:
    case CPE_DR_TYPE_UINT32:
        return *(uint32_t *)data;
    case CPE_DR_TYPE_INT64:
    case CPE_DR_TYPE_UINT64:
        return (*(uint64_t *)data) & 0xFFFFFFFF;
    case CPE_DR_TYPE_FLOAT:
    case CPE_DR_TYPE_DOUBLE:
        return (*(uint64_t *)data) & 0xFFFFFFFF;
    case CPE_DR_TYPE_STRING:
        return cpe_hash_str(data, strlen(data));
    case CPE_DR_TYPE_DATE:
    case CPE_DR_TYPE_TIME:
    case CPE_DR_TYPE_DATETIME:
    case CPE_DR_TYPE_MONEY:
    case CPE_DR_TYPE_IP:
    case CPE_DR_TYPE_VOID:
    default:
        return entry->m_type;
    }
}

uint32_t dr_meta_key_hash(const void * input, LPDRMETA meta) {
    int i;
    uint32_t value;
    int key_num = dr_meta_key_entry_num(meta);

    value = 5381;
    for(i = 0; i < key_num; ++i) {
        LPDRMETAENTRY entry = dr_meta_key_entry_at(meta, i);
        uint32_t entry_value = dr_entry_hash(((const char *)input) + entry->m_data_start_pos, entry);
        value = value ^ ((value << 5) + (value >> 2) + entry_value);
    }

    return value;
}

int dr_meta_key_cmp(const void * l, const void * r, LPDRMETA meta) {
    int i;
    int key_num = dr_meta_key_entry_num(meta);

    for(i = 0; i < key_num; ++i) {
        LPDRMETAENTRY entry = dr_meta_key_entry_at(meta, i);

        int rv = dr_entry_cmp(
            ((const char *)l) + entry->m_data_start_pos,
            ((const char *)r) + entry->m_data_start_pos,
            entry);
        if (rv) return rv;
    }

    return 0;
}

int dr_meta_data_cmp(const void * l, const void * r, LPDRMETA meta) {
    int i;
    int num = dr_meta_entry_num(meta);

    for(i = 0; i < num; ++i) {
        LPDRMETAENTRY entry = dr_meta_entry_at(meta, i);

        int rv = dr_entry_cmp(
            ((const char *)l) + entry->m_data_start_pos,
            ((const char *)r) + entry->m_data_start_pos,
            entry);
        if (rv) return rv;
    }

    return 0;
}

