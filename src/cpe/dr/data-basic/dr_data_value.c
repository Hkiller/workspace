#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_data_value.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"

int8_t dr_value_read_with_dft_int8(dr_value_t value, int8_t dft) {
    int8_t r;
    return dr_ctype_try_read_int8(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

uint8_t dr_value_read_with_dft_uint8(dr_value_t value, uint8_t dft) {
    uint8_t r;
    return dr_ctype_try_read_uint8(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

int16_t dr_value_read_with_dft_int16(dr_value_t value, int16_t dft) {
    int16_t r;
    return dr_ctype_try_read_int16(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

uint16_t dr_value_read_with_dft_uint16(dr_value_t value, uint16_t dft) {
    uint16_t r;
    return dr_ctype_try_read_uint16(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

int32_t dr_value_read_with_dft_int32(dr_value_t value, int32_t dft) {
    int32_t r;
    return dr_ctype_try_read_int32(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

uint32_t dr_value_read_with_dft_uint32(dr_value_t value, uint32_t dft) {
    uint32_t r;
    return dr_ctype_try_read_uint32(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

int64_t dr_value_read_with_dft_int64(dr_value_t value, int64_t dft) {
    int64_t r;
    return dr_ctype_try_read_int64(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

uint64_t dr_value_read_with_dft_uint64(dr_value_t value, uint64_t dft) {
    uint64_t r;
    return dr_ctype_try_read_uint64(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

float dr_value_read_with_dft_float(dr_value_t value, float dft) {
    float r;
    return dr_ctype_try_read_float(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

double dr_value_read_with_dft_double(dr_value_t value, double dft) {
    double r;
    return dr_ctype_try_read_double(&r, value->m_data, value->m_type, NULL) == 0 ? r : dft;
}

const char * dr_value_read_with_dft_string(dr_value_t value, const char * dft) {
    return (value->m_type == CPE_DR_TYPE_STRING) ? value->m_data : dft;
}

const char * dr_value_to_string(mem_buffer_t buf, dr_value_t value, const char * dft) {
    if (value->m_type == CPE_DR_TYPE_STRING) {
        return value->m_data;
    }
    else {
        struct write_stream_buffer s = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buf);
        mem_buffer_clear_data(buf);
        if (dr_ctype_print_to_stream((write_stream_t)&s, value->m_data, value->m_type, NULL) < 0) {
            return dft;
        }

        stream_putc((write_stream_t)&s, 0);
        
        return mem_buffer_make_continuous(buf, 0);
    }
}

int dr_value_try_read_int8(int8_t * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_int8(result, value->m_data, value->m_type, em);
}

int dr_value_try_read_uint8(uint8_t * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_uint8(result, value->m_data, value->m_type, em);
}

int dr_value_try_read_int16(int16_t * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_int16(result, value->m_data, value->m_type, em);
}

int dr_value_try_read_uint16(uint16_t * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_uint16(result, value->m_data, value->m_type, em);
}

int dr_value_try_read_int32(int32_t * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_int32(result, value->m_data, value->m_type, em);
}

int dr_value_try_read_uint32(uint32_t * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_uint32(result, value->m_data, value->m_type, em);
}

int dr_value_try_read_int64(int64_t * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_int64(result, value->m_data, value->m_type, em);
}

int dr_value_try_read_uint64(uint64_t * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_uint64(result, value->m_data, value->m_type, em);
}

int dr_value_try_read_float(float * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_float(result, value->m_data, value->m_type, em);
}

int dr_value_try_read_double(double * result, dr_value_t value, error_monitor_t em) {
    return dr_ctype_try_read_double(result, value->m_data, value->m_type, em);
}

const char * dr_value_try_read_string(dr_value_t value, error_monitor_t em) {
    return (value->m_type == CPE_DR_TYPE_STRING) ? value->m_data : NULL;
}

int dr_value_set_from_value(dr_value_t to, dr_value_t from, error_monitor_t em) {
    if (to->m_meta) {
        if (from->m_meta == NULL) {
            CPE_ERROR(em, "set value %s from basic value fail", dr_meta_name(to->m_meta));
            return -1;
        }
        
        if (dr_meta_copy_same_entry(
                to->m_data, to->m_size, to->m_meta,
                from->m_data, from->m_size, from->m_meta,
                0, em)
            <= 0)
        {
            CPE_ERROR(
                em, "set value %s from %s: copy same entry error",
                dr_meta_name(to->m_meta), dr_meta_name(from->m_meta));
            return -1;
        }

        return 0;
    }

    assert(to->m_type > CPE_DR_TYPE_COMPOSITE);
    
    if (from->m_type <= CPE_DR_TYPE_COMPOSITE) {
        CPE_ERROR(
            em, "set value %s from %s: type convert error",
            dr_type_name(to->m_type), dr_meta_name(from->m_meta));
        return -1;
    }

    if (to->m_type == CPE_DR_TYPE_STRING) {
        if (from->m_type == CPE_DR_TYPE_STRING) {
            size_t len = strlen(from->m_data);
            if (len + 1 > from->m_size) {
                len = from->m_size - 1;
            }

            memcpy(to->m_data, from->m_data, len);
            ((char*)to->m_data)[len] = 0;
        }
        else {
            struct write_stream_mem ws = CPE_WRITE_STREAM_MEM_INITIALIZER(to->m_data, to->m_size - 1);
            if (dr_ctype_print_to_stream((write_stream_t)&ws, from->m_data, from->m_type, em) != 0) {
                CPE_ERROR(
                    em, "set value %s from %s error",
                    dr_type_name(to->m_type), dr_type_name(from->m_type));
                return -1;
            }
            ((char*)to->m_data)[ws.m_pos] = 0;
        }
    }
    else {
        if (dr_ctype_set_from_ctype(to->m_data, to->m_type, from->m_type, from->m_data, em) != 0) {
            CPE_ERROR(
                em, "set value %s from %s error",
                dr_type_name(to->m_type), dr_type_name(from->m_type));
            return -1;
        }
    }
    
    return 0;
}

static int dr_value_adj_by_entry(dr_value_t value_buf, char * base, LPDRMETAENTRY entry) {
    uint8_t is_last_entry;

    assert(value_buf->m_meta);

    is_last_entry =
        entry == dr_meta_entry_at(value_buf->m_meta, dr_meta_entry_num(value_buf->m_meta) - 1)
        ? 1 : 0;

    value_buf->m_type = entry->m_type;
    if (entry->m_type > CPE_DR_TYPE_COMPOSITE) {
        value_buf->m_meta = NULL;
    }
    else {
        value_buf->m_meta = (LPDRMETA)(base + entry->m_ref_type_pos);
    }

    if (entry->m_data_start_pos > value_buf->m_size) return -1;
                
    value_buf->m_data = ((char*)value_buf->m_data) + entry->m_data_start_pos;
    value_buf->m_size -= entry->m_data_start_pos;

    if (!is_last_entry || entry->m_array_count != 0) {
        size_t entry_size = dr_entry_size(entry);
        if (value_buf->m_size > entry_size) value_buf->m_size = entry_size;
    }
    
    return 0;
}

static int dr_value_adj_by_pos(dr_value_t value_buf, int array_pos) {
    int element_size;
    int start_pos;
    
    if (value_buf->m_meta) {
        element_size = (int)dr_meta_size(value_buf->m_meta);
    }
    else {
        element_size = dr_type_size(value_buf->m_type);
    }

    start_pos = element_size * array_pos;

    if (start_pos > value_buf->m_size) return -1;
                
    value_buf->m_data = ((char*)value_buf->m_data) + start_pos;
    value_buf->m_size -= start_pos;

    if (value_buf->m_size > element_size) value_buf->m_size = element_size;

    return 0;
}

dr_value_t dr_value_find_by_path(dr_data_t i_data, const char * path, dr_value_t value_buf) {
    char * base;
    LPDRMETAENTRY entry = NULL;
    const char * check_begin;
    const char * point_pos;
    const char * array_pos;

    assert(path);

    base = i_data->m_meta ? ((char *)(i_data->m_meta) - i_data->m_meta->m_self_pos) : NULL;
    
    value_buf->m_type = dr_meta_type(i_data->m_meta);
    value_buf->m_meta = i_data->m_meta;
    value_buf->m_data = i_data->m_data;
    value_buf->m_size = i_data->m_size;

    check_begin = path;
    point_pos = strchr(check_begin, '.');
    array_pos = strchr(check_begin, '[');

    while(array_pos) {
        if (*check_begin == '[') {
            char * array_end;
            int pos;
            
            assert(check_begin == array_pos);

            pos = (int)strtol(check_begin + 1, &array_end, 10);
            if (array_end == NULL || *array_end != ']') return NULL;

            if (dr_value_adj_by_pos(value_buf, pos) != 0) return NULL;
            
            check_begin = array_end + 1;

            if (*check_begin == '.') {
                check_begin++;
                point_pos = *check_begin ? strchr(check_begin, '.') : NULL;
            }

            array_pos = *check_begin ? strchr(check_begin, '[') : NULL;
        }
        else {
            if (value_buf->m_meta == NULL) return NULL;

            if (point_pos && point_pos < array_pos) {
                entry = dr_meta_find_entry_by_name_len(value_buf->m_meta, check_begin, point_pos - check_begin);
                if (entry == NULL) return NULL;

                if (dr_value_adj_by_entry(value_buf, base, entry) != 0) return NULL;

                check_begin = point_pos + 1;
                point_pos = *check_begin ? strchr(check_begin, '.') : NULL;
            }
            else {
                entry = dr_meta_find_entry_by_name_len(value_buf->m_meta, check_begin, array_pos - check_begin);
                if (entry == NULL) return NULL;

                if (entry->m_array_count == 1) return NULL;
                if (dr_value_adj_by_entry(value_buf, base, entry) != 0) return NULL;

                check_begin = array_pos;
            }
        }
    }

    while(point_pos) {
        if (value_buf->m_meta == NULL) return NULL;

        entry = dr_meta_find_entry_by_name_len(value_buf->m_meta, check_begin, point_pos - check_begin);
        if (entry == NULL) return NULL;

        if (dr_value_adj_by_entry(value_buf, base, entry) != 0) return NULL;
        
        check_begin = point_pos + 1;
        point_pos = *check_begin ? strchr(check_begin, '.') : NULL;
    }

    if (*check_begin) {
        if (value_buf->m_meta == NULL) return NULL;

        entry = dr_meta_find_entry_by_name(value_buf->m_meta, check_begin);
        if (entry == NULL) return NULL;

        if (dr_value_adj_by_entry(value_buf, base, entry) != 0) return NULL;
    }

    return value_buf;
}
