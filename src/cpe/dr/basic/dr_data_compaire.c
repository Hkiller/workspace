#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/math_ex.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "../dr_internal_types.h"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4244)
#endif

#define __value(t, v) (*(t const *)v)
#define __value_cast(ct, t, v) ((ct)__value(t, v))

int dr_ctype_cmp(const void * l, int l_type, const void * r, int r_type) {

    assert(l);
    assert(r);
    assert(l_type > CPE_DR_TYPE_COMPOSITE);
    assert(l_type <= CPE_DR_TYPE_MAX);
    assert(r_type > CPE_DR_TYPE_COMPOSITE);
    assert(r_type <= CPE_DR_TYPE_MAX);

    switch(l_type) {
    case CPE_DR_TYPE_CHAR:
    case CPE_DR_TYPE_INT8: {
        switch(r_type) {
        case CPE_DR_TYPE_CHAR:
        case CPE_DR_TYPE_INT8:
            return cpe_cmp(__value(int8_t, l), __value(int8_t, r));
        case CPE_DR_TYPE_UINT8:
        case CPE_DR_TYPE_UCHAR:
            return cpe_cmp(__value(uint8_t, l), __value(uint8_t, r));
        case CPE_DR_TYPE_INT16:
            return cpe_cmp(__value_cast(int16_t, int8_t, l), __value(int16_t, r));
        case CPE_DR_TYPE_UINT16:
            return cpe_cmp(__value_cast(uint16_t, int8_t, l), __value(uint16_t, r));
        case CPE_DR_TYPE_INT32:
            return cpe_cmp(__value_cast(int32_t, int8_t, l), __value(int32_t, r));
        case CPE_DR_TYPE_UINT32:
            return cpe_cmp(__value_cast(uint32_t, int8_t, l), __value(uint32_t, r));
        case CPE_DR_TYPE_INT64:
            return cpe_cmp(__value_cast(int64_t, int8_t, l), __value(int64_t, r));
        case CPE_DR_TYPE_UINT64:
            return cpe_cmp(__value_cast(uint64_t, int8_t, l), __value(uint64_t, r));
        case CPE_DR_TYPE_FLOAT:
            return cpe_cmp(__value_cast(float, int8_t, l), __value(float, r));
        case CPE_DR_TYPE_DOUBLE:
            return cpe_cmp(__value_cast(double, int8_t, l), __value(double, r));
        case CPE_DR_TYPE_STRING:
            if (l_type == CPE_DR_TYPE_CHAR) {
                if (__value(int8_t, l) == __value(int8_t, r)) {
                    if (__value(int8_t, l) == 0) return 0;
                
                    if (__value(int8_t, ((char*)r) + 1) == 0) return 0;
                    else return -1;
                }
                else {
                    return cpe_cmp(__value(int8_t, l), __value(int8_t, r));
                }
            }
            else {
                int64_t buf;
                if (dr_ctype_try_read_int64(&buf, r, r_type, 0) == 0) {
                    return cpe_cmp(__value_cast(int64_t, int8_t, l), buf);
                }
                else {
                    return l_type - r_type;
                }
            }
        default:
            break;
        }
        break;
    }
    case CPE_DR_TYPE_UCHAR:
    case CPE_DR_TYPE_UINT8: {
        switch(r_type) {
        case CPE_DR_TYPE_CHAR:
        case CPE_DR_TYPE_INT8:
            return cpe_cmp(__value(uint8_t, l), __value(uint8_t, r));
        case CPE_DR_TYPE_UINT8:
        case CPE_DR_TYPE_UCHAR:
            return cpe_cmp(__value(uint8_t, l), __value(uint8_t, r));
        case CPE_DR_TYPE_INT16:
            return cpe_cmp(__value_cast(int16_t, uint8_t, l), __value(int16_t, r));
        case CPE_DR_TYPE_UINT16:
            return cpe_cmp(__value_cast(uint16_t, uint8_t, l), __value(uint16_t, r));
        case CPE_DR_TYPE_INT32:
            return cpe_cmp(__value_cast(int32_t, uint8_t, l), __value(int32_t, r));
        case CPE_DR_TYPE_UINT32:
            return cpe_cmp(__value_cast(uint32_t, uint8_t, l), __value(uint32_t, r));
        case CPE_DR_TYPE_INT64:
            return cpe_cmp(__value_cast(int64_t, uint8_t, l), __value(int64_t, r));
        case CPE_DR_TYPE_UINT64:
            return cpe_cmp(__value_cast(uint64_t, uint8_t, l), __value(uint64_t, r));
        case CPE_DR_TYPE_FLOAT:
            return cpe_cmp(__value_cast(float, uint8_t, l), __value(float, r));
        case CPE_DR_TYPE_DOUBLE:
            return cpe_cmp(__value_cast(double, uint8_t, l), __value(double, r));
        case CPE_DR_TYPE_STRING:
            if (l_type == CPE_DR_TYPE_UCHAR) {
                if (__value(uint8_t, l) == __value(uint8_t, r)) {
                    if (__value(uint8_t, l) == 0) return 0;
                
                    if (__value(uint8_t, ((char*)r) + 1) == 0) return 0;
                    else return -1;
                }
                else {
                    return cpe_cmp(__value(uint8_t, l), __value(uint8_t, r));
                }
            }
            else {
                uint64_t buf;
                if (dr_ctype_try_read_uint64(&buf, r, r_type, 0) == 0) {
                    return cpe_cmp(__value_cast(uint64_t, uint8_t, l), buf);
                }
                else {
                    return l_type - r_type;
                }
            }
        default:
            break;
        }
        break;
    }
    case CPE_DR_TYPE_INT16: {
        switch(r_type) {
        case CPE_DR_TYPE_CHAR:
        case CPE_DR_TYPE_INT8:
            return cpe_cmp(__value(int16_t, l), __value_cast(int16_t, int8_t, r));
        case CPE_DR_TYPE_UINT8:
        case CPE_DR_TYPE_UCHAR:
            return cpe_cmp(__value(uint16_t, l), __value_cast(uint16_t, uint8_t, r));
        case CPE_DR_TYPE_INT16:
            return cpe_cmp(__value(int16_t, l), __value(int16_t, r));
        case CPE_DR_TYPE_UINT16:
            return cpe_cmp(__value(uint16_t, l), __value(uint16_t, r));
        case CPE_DR_TYPE_INT32:
            return cpe_cmp(__value_cast(int32_t, int16_t, l), __value(int32_t, r));
        case CPE_DR_TYPE_UINT32:
            return cpe_cmp(__value_cast(uint32_t, int16_t, l), __value(uint32_t, r));
        case CPE_DR_TYPE_INT64:
            return cpe_cmp(__value_cast(int64_t, int16_t, l), __value(int64_t, r));
        case CPE_DR_TYPE_UINT64:
            return cpe_cmp(__value_cast(uint64_t, int16_t, l), __value(uint64_t, r));
        case CPE_DR_TYPE_FLOAT:
            return cpe_cmp(__value_cast(float, int16_t, l), __value(float, r));
        case CPE_DR_TYPE_DOUBLE:
            return cpe_cmp(__value_cast(double, int16_t, l), __value(double, r));
        case CPE_DR_TYPE_STRING: {
            int64_t buf;
            if (dr_ctype_try_read_int64(&buf, r, r_type, 0) == 0) {
                return cpe_cmp(__value_cast(int64_t, int16_t, l), buf);
            }
            else {
                return l_type - r_type;
            }
        }
        default:
            break;
        }
        break;
    }
    case CPE_DR_TYPE_UINT16: {
        switch(r_type) {
        case CPE_DR_TYPE_CHAR:
        case CPE_DR_TYPE_INT8:
            return cpe_cmp(__value(uint16_t, l), __value_cast(uint16_t, uint8_t, r));
        case CPE_DR_TYPE_UINT8:
        case CPE_DR_TYPE_UCHAR:
            return cpe_cmp(__value(uint16_t, l), __value_cast(uint16_t, uint8_t, r));
        case CPE_DR_TYPE_INT16:
            return cpe_cmp(__value(uint16_t, l), __value(uint16_t, r));
        case CPE_DR_TYPE_UINT16:
            return cpe_cmp(__value(uint16_t, l), __value(uint16_t, r));
        case CPE_DR_TYPE_INT32:
            return cpe_cmp(__value_cast(int32_t, uint16_t, l), __value(int32_t, r));
        case CPE_DR_TYPE_UINT32:
            return cpe_cmp(__value_cast(uint32_t, uint16_t, l), __value(uint32_t, r));
        case CPE_DR_TYPE_INT64:
            return cpe_cmp(__value_cast(int64_t, uint16_t, l), __value(int64_t, r));
        case CPE_DR_TYPE_UINT64:
            return cpe_cmp(__value_cast(uint64_t, uint16_t, l), __value(uint64_t, r));
        case CPE_DR_TYPE_FLOAT:
            return cpe_cmp(__value_cast(float, uint16_t, l), __value(float, r));
        case CPE_DR_TYPE_DOUBLE:
            return cpe_cmp(__value_cast(double, uint16_t, l), __value(double, r));
        case CPE_DR_TYPE_STRING: {
            uint64_t buf;
            if (dr_ctype_try_read_uint64(&buf, r, r_type, 0) == 0) {
                return cpe_cmp(__value_cast(uint64_t, uint16_t, l), buf);
            }
            else {
                return l_type - r_type;
            }
        }
        default:
            break;
        }
        break;
    }
    case CPE_DR_TYPE_INT32: {
        switch(r_type) {
        case CPE_DR_TYPE_CHAR:
        case CPE_DR_TYPE_INT8:
            return cpe_cmp(__value(int32_t, l), __value_cast(int32_t, int8_t, r));
        case CPE_DR_TYPE_UINT8:
        case CPE_DR_TYPE_UCHAR:
            return cpe_cmp(__value(uint32_t, l), __value_cast(uint32_t, uint8_t, r));
        case CPE_DR_TYPE_INT16:
            return cpe_cmp(__value(int32_t, l), __value_cast(int32_t, int16_t, r));
        case CPE_DR_TYPE_UINT16:
            return cpe_cmp(__value(uint32_t, l), __value_cast(uint32_t, uint16_t, r));
        case CPE_DR_TYPE_INT32:
            return cpe_cmp(__value(int32_t, l), __value(int32_t, r));
        case CPE_DR_TYPE_UINT32:
            return cpe_cmp(__value(uint32_t, l), __value(uint32_t, r));
        case CPE_DR_TYPE_INT64:
            return cpe_cmp(__value_cast(int64_t, int32_t, l), __value(int64_t, r));
        case CPE_DR_TYPE_UINT64:
            return cpe_cmp(__value_cast(uint64_t, int32_t, l), __value(uint64_t, r));
        case CPE_DR_TYPE_FLOAT:
            return cpe_cmp(__value_cast(float, int32_t, l), __value(float, r));
        case CPE_DR_TYPE_DOUBLE:
            return cpe_cmp(__value_cast(double, int32_t, l), __value(double, r));
        case CPE_DR_TYPE_STRING: {
            int64_t buf;
            if (dr_ctype_try_read_int64(&buf, r, r_type, 0) == 0) {
                return cpe_cmp(__value_cast(int64_t, int32_t, l), buf);
            }
            else {
                return l_type - r_type;
            }
        }
        default:
            break;
        }
        break;
    }
    case CPE_DR_TYPE_UINT32: {
        switch(r_type) {
        case CPE_DR_TYPE_CHAR:
        case CPE_DR_TYPE_INT8:
            return cpe_cmp(__value(uint32_t, l), __value_cast(uint32_t, uint8_t, r));
        case CPE_DR_TYPE_UINT8:
        case CPE_DR_TYPE_UCHAR:
            return cpe_cmp(__value(uint32_t, l), __value_cast(uint32_t, uint8_t, r));
        case CPE_DR_TYPE_INT16:
            return cpe_cmp(__value(uint32_t, l), __value_cast(uint32_t, uint16_t, r));
        case CPE_DR_TYPE_UINT16:
            return cpe_cmp(__value(uint32_t, l), __value_cast(uint32_t, uint16_t, r));
        case CPE_DR_TYPE_INT32:
            return cpe_cmp(__value(uint32_t, l), __value(uint32_t, r));
        case CPE_DR_TYPE_UINT32:
            return cpe_cmp(__value(uint32_t, l), __value(uint32_t, r));
        case CPE_DR_TYPE_INT64:
            return cpe_cmp(__value_cast(int64_t, uint32_t, l), __value(int64_t, r));
        case CPE_DR_TYPE_UINT64:
            return cpe_cmp(__value_cast(uint64_t, uint32_t, l), __value(uint64_t, r));
        case CPE_DR_TYPE_FLOAT:
            return cpe_cmp(__value_cast(float, uint32_t, l), __value(float, r));
        case CPE_DR_TYPE_DOUBLE:
            return cpe_cmp(__value_cast(double, uint32_t, l), __value(double, r));
        case CPE_DR_TYPE_STRING: {
            uint64_t buf;
            if (dr_ctype_try_read_uint64(&buf, r, r_type, 0) == 0) {
                return cpe_cmp(__value_cast(uint64_t, uint32_t, l), buf);
            }
            else {
                return l_type - r_type;
            }
        }
        default:
            break;
        }
        break;
    }
    case CPE_DR_TYPE_INT64: {
        switch(r_type) {
        case CPE_DR_TYPE_CHAR:
        case CPE_DR_TYPE_INT8:
            return cpe_cmp(__value(int64_t, l), __value_cast(int64_t, int8_t, r));
        case CPE_DR_TYPE_UINT8:
        case CPE_DR_TYPE_UCHAR:
            return cpe_cmp(__value(uint64_t, l), __value_cast(uint64_t, uint8_t, r));
        case CPE_DR_TYPE_INT16:
            return cpe_cmp(__value(int64_t, l), __value_cast(int64_t, int16_t, r));
        case CPE_DR_TYPE_UINT16:
            return cpe_cmp(__value(uint64_t, l), __value_cast(uint64_t, uint16_t, r));
        case CPE_DR_TYPE_INT32:
            return cpe_cmp(__value(int64_t, l), __value_cast(int64_t, int32_t, r));
        case CPE_DR_TYPE_UINT32:
            return cpe_cmp(__value(uint64_t, l), __value_cast(uint64_t, uint32_t, r));
        case CPE_DR_TYPE_INT64:
            return cpe_cmp(__value(int64_t, l), __value(int64_t, r));
        case CPE_DR_TYPE_UINT64:
            return cpe_cmp(__value(uint64_t, l), __value(uint64_t, r));
        case CPE_DR_TYPE_FLOAT:
            return cpe_cmp(__value_cast(float, int64_t, l), __value(float, r));
        case CPE_DR_TYPE_DOUBLE:
            return cpe_cmp(__value_cast(double, int64_t, l), __value(double, r));
        case CPE_DR_TYPE_STRING: {
            int64_t buf;
            if (dr_ctype_try_read_int64(&buf, r, r_type, 0) == 0) {
                return cpe_cmp(__value(int64_t, l), buf);
            }
            else {
                return l_type - r_type;
            }
        }
        default:
            break;
        }
        break;
    }
    case CPE_DR_TYPE_UINT64: {
        switch(r_type) {
        case CPE_DR_TYPE_CHAR:
        case CPE_DR_TYPE_INT8:
            return cpe_cmp(__value(uint64_t, l), __value_cast(uint64_t, uint8_t, r));
        case CPE_DR_TYPE_UINT8:
        case CPE_DR_TYPE_UCHAR:
            return cpe_cmp(__value(uint64_t, l), __value_cast(uint64_t, uint8_t, r));
        case CPE_DR_TYPE_INT16:
            return cpe_cmp(__value(uint64_t, l), __value_cast(uint64_t, uint16_t, r));
        case CPE_DR_TYPE_UINT16:
            return cpe_cmp(__value(uint64_t, l), __value_cast(uint64_t, uint16_t, r));
        case CPE_DR_TYPE_INT32:
            return cpe_cmp(__value(uint64_t, l), __value_cast(uint64_t, uint32_t, r));
        case CPE_DR_TYPE_UINT32:
            return cpe_cmp(__value(uint64_t, l), __value_cast(uint64_t, uint32_t, r));
        case CPE_DR_TYPE_INT64:
            return cpe_cmp(__value(int64_t, r), __value(int64_t, r));
        case CPE_DR_TYPE_UINT64:
            return cpe_cmp(__value(uint64_t, l), __value(uint64_t, r));
        case CPE_DR_TYPE_FLOAT:
            return (int)(__value_cast(float, uint64_t, l) - __value(float, r));
        case CPE_DR_TYPE_DOUBLE:
            return (int)(__value_cast(double, uint64_t, l) - __value(double, r));
        case CPE_DR_TYPE_STRING: {
            uint64_t buf;
            if (dr_ctype_try_read_uint64(&buf, r, r_type, 0) == 0) {
                return cpe_cmp(__value(uint64_t, l), buf);
            }
            else {
                return l_type - r_type;
            }
        }
        default:
            break;
        }
        break;
    }
    case CPE_DR_TYPE_FLOAT: {
        switch(r_type) {
        case CPE_DR_TYPE_CHAR:
        case CPE_DR_TYPE_INT8:
            return cpe_cmp(__value(float, l), __value_cast(float, int8_t, r));
        case CPE_DR_TYPE_UINT8:
        case CPE_DR_TYPE_UCHAR:
            return cpe_cmp(__value(float, l), __value_cast(float, uint8_t, r));
        case CPE_DR_TYPE_INT16:
            return (int)(__value(float, l) - __value_cast(float, int16_t, r));
        case CPE_DR_TYPE_UINT16:
            return (int)(__value(float, l) - __value_cast(float, uint16_t, r));
        case CPE_DR_TYPE_INT32:
            return (int)(__value(float, l) - __value_cast(float, int32_t, r));
        case CPE_DR_TYPE_UINT32:
            return (int)(__value(float, l) - __value_cast(float, uint32_t, r));
        case CPE_DR_TYPE_INT64:
            return (int)(__value(float, l) - __value_cast(float, int64_t, r));
        case CPE_DR_TYPE_UINT64:
            return (int)(__value(float, l) - __value_cast(float, uint64_t, r));
        case CPE_DR_TYPE_FLOAT:
            return (int)(__value(float, l) - __value(float, r));
        case CPE_DR_TYPE_DOUBLE:
            return (int)(__value_cast(double, float, l) - __value(double, r));
        case CPE_DR_TYPE_STRING: {
            float buf;
            if (dr_ctype_try_read_float(&buf, r, r_type, 0) == 0) {
                return __value(float, l) - buf;
            }
            else {
                return l_type - r_type;
            }
        }
        default:
            break;
        }
        break;
    }
    case CPE_DR_TYPE_DOUBLE: {
        switch(r_type) {
        case CPE_DR_TYPE_CHAR:
        case CPE_DR_TYPE_INT8:
            return __value(double, l) - __value_cast(double, int8_t, r);
        case CPE_DR_TYPE_UINT8:
        case CPE_DR_TYPE_UCHAR:
            return __value(double, l) - __value_cast(double, uint8_t, r);
        case CPE_DR_TYPE_INT16:
            return __value(double, l) - __value_cast(double, int16_t, r);
        case CPE_DR_TYPE_UINT16:
            return __value(double, l) - __value_cast(double, uint16_t, r);
        case CPE_DR_TYPE_INT32:
            return __value(double, l) - __value_cast(double, int32_t, r);
        case CPE_DR_TYPE_UINT32:
            return __value(double, l) - __value_cast(double, uint32_t, r);
        case CPE_DR_TYPE_INT64:
            return __value(double, l) - __value_cast(double, int64_t, r);
        case CPE_DR_TYPE_UINT64:
            return __value(double, l) - __value_cast(double, uint64_t, r);
        case CPE_DR_TYPE_FLOAT:
            return __value(double, l) - __value_cast(double, float, r);
        case CPE_DR_TYPE_DOUBLE:
            return __value(double, l) - __value(double, r);
        case CPE_DR_TYPE_STRING: {
            double buf;
            if (dr_ctype_try_read_double(&buf, r, r_type, 0) == 0) {
                return __value(double, l) - buf;
            }
            else {
                return l_type - r_type;
            }
        }
        default:
            break;
        }
        break;
    }
    case CPE_DR_TYPE_STRING: {
        switch(r_type) {
        case CPE_DR_TYPE_CHAR: {
        }
        case CPE_DR_TYPE_UCHAR: {
        }
        case CPE_DR_TYPE_INT8: {
            int64_t buf;
            if (dr_ctype_try_read_int64(&buf, l, l_type, 0) == 0) {
                return (int)(buf - __value_cast(int64_t, int8_t, r));
            }
            else {
                return l_type - r_type;
            }
        }
        case CPE_DR_TYPE_INT16: {
            int64_t buf;
            if (dr_ctype_try_read_int64(&buf, l, l_type, 0) == 0) {
                return (int)(buf - __value_cast(int64_t, int16_t, r));
            }
            else {
                return l_type - r_type;
            }
        }
        case CPE_DR_TYPE_INT32: {
            int64_t buf;
            if (dr_ctype_try_read_int64(&buf, l, l_type, 0) == 0) {
                return (int)(buf - __value_cast(int64_t, int32_t, r));
            }
            else {
                return l_type - r_type;
            }
        }
        case CPE_DR_TYPE_INT64: {
            int64_t buf;
            if (dr_ctype_try_read_int64(&buf, l, l_type, 0) == 0) {
                return (int)(buf - __value(int64_t, r));
            }
            else {
                return l_type - r_type;
            }
        }
        case CPE_DR_TYPE_UINT8: {
            uint64_t buf;
            if (dr_ctype_try_read_uint64(&buf, l, l_type, 0) == 0) {
                return (int)(buf - __value_cast(uint64_t, uint8_t, r));
            }
            else {
                return l_type - r_type;
            }
        }
        case CPE_DR_TYPE_UINT16: {
            uint64_t buf;
            if (dr_ctype_try_read_uint64(&buf, l, l_type, 0) == 0) {
                return (int)(buf - __value_cast(uint64_t, uint16_t, r));
            }
            else {
                return l_type - r_type;
            }
        }
        case CPE_DR_TYPE_UINT32: {
            uint64_t buf;
            if (dr_ctype_try_read_uint64(&buf, l, l_type, 0) == 0) {
                return (int)(buf - __value_cast(uint64_t, uint32_t, r));
            }
            else {
                return l_type - r_type;
            }
        }
        case CPE_DR_TYPE_UINT64: {
            uint64_t buf;
            if (dr_ctype_try_read_uint64(&buf, l, l_type, 0) == 0) {
                return (int)(buf - __value(uint64_t, r));
            }
            else {
                return l_type - r_type;
            }
        }
        case CPE_DR_TYPE_FLOAT: {
            double buf;
            if (dr_ctype_try_read_double(&buf, l, l_type, 0) == 0) {
                return (int)(buf - __value_cast(double, float, r));
            }
            else {
                return l_type - r_type;
            }
        }
        case CPE_DR_TYPE_DOUBLE: {
            double buf;
            if (dr_ctype_try_read_double(&buf, l, l_type, 0) == 0) {
                return (int)(buf - __value(double, r));
            }
            else {
                return l_type - r_type;
            }
        }
        case CPE_DR_TYPE_STRING:
            return strcmp((const char *)l, (const char *)r);
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    if (l_type == r_type) {
        return memcmp(l, r, dr_type_size(l_type));
    }
    else {
        return l_type - r_type;
    }
}

int dr_entry_cmp(const void * l, const void * r, LPDRMETAENTRY entry) {
    const char * l_data;
    const char * r_data;

    l_data = ((const char *)l) + entry->m_data_start_pos;
    r_data = ((const char *)r) + entry->m_data_start_pos;

    switch(entry->m_type) {
    case CPE_DR_TYPE_UNION: {
        LPDRMETAENTRY select_entry;
        select_entry = dr_entry_select_entry(entry);
        if (select_entry) {
            int32_t l_union_entry_id;
            int32_t r_union_entry_id;
            int32_t use_entry_pos;

            dr_entry_try_read_int32(
                &l_union_entry_id,
                ((const char *)l) + entry->m_select_data_start_pos,
                select_entry,
                NULL);

            dr_entry_try_read_int32(
                &r_union_entry_id,
                ((const char *)r) + entry->m_select_data_start_pos,
                select_entry,
                NULL);

            if (l_union_entry_id != r_union_entry_id) {
                return l_union_entry_id - r_union_entry_id;
            }

            use_entry_pos = dr_meta_find_entry_idx_by_id(dr_entry_self_meta(entry), l_union_entry_id);
            if (use_entry_pos >= 0) {
                return dr_entry_cmp(l_data, r_data, dr_meta_entry_at(dr_entry_self_meta(entry), use_entry_pos));
            }
        }

        return memcmp(l_data, r_data, dr_entry_size(entry));
    }
    case CPE_DR_TYPE_STRUCT: {
        LPDRMETA meta;
        int i;
        size_t count;
        int r;

        meta = dr_entry_ref_meta(entry);
        if (meta == NULL) return memcmp(l_data, r_data, dr_entry_size(entry));

        count = dr_meta_entry_num(meta);
        for(i = 0; i < count; ++i) {
            r = dr_entry_cmp(l_data, r_data, dr_meta_entry_at(meta, i));
            if (r != 0) return r;
        }

        return 0;
    }
    default:
        return dr_ctype_cmp(l_data, entry->m_type, r_data, entry->m_type);
    }
}

#ifdef _MSC_VER
# pragma warning(pop)
#endif
