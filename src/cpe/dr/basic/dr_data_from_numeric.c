#include <ctype.h>
#include <limits.h>
#include <string.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_data.h"
#include "../dr_internal_types.h"

typedef float float_t;
typedef double double_t;

struct DRCtypeTypeWriteOps {
    int (*from_int8)(void * result, int8_t input, LPDRMETAENTRY entry, error_monitor_t em);
    int (*from_uint8)(void * result, uint8_t input, LPDRMETAENTRY entry, error_monitor_t em);
    int (*from_int16)(void * result, int16_t input, LPDRMETAENTRY entry, error_monitor_t em);
    int (*from_uint16)(void * result, uint16_t input, LPDRMETAENTRY entry, error_monitor_t em);
    int (*from_int32)(void * result, int32_t input, LPDRMETAENTRY entry, error_monitor_t em);
    int (*from_uint32)(void * result, uint32_t input, LPDRMETAENTRY entry, error_monitor_t em);
    int (*from_int64)(void * result, int64_t input, LPDRMETAENTRY entry, error_monitor_t em);
    int (*from_uint64)(void * result, uint64_t input, LPDRMETAENTRY entry, error_monitor_t em);
    int (*from_float)(void * result, float input, LPDRMETAENTRY entry, error_monitor_t em);
    int (*from_double)(void * result, double input, LPDRMETAENTRY entry, error_monitor_t em);
};

#define DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(__to, __from)             \
int __to ## _from_ ## __from(void * result, __from ## _t  input,          \
                           LPDRMETAENTRY entry, error_monitor_t em) {   \
    *((__to ## _t *)result) = (__to ## _t)input;                        \
    return 0;                                                           \
}


#define DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MIN(__to, __from, __min, __valuefmt)   \
int __to ## _from_ ## __from(void * result, __from ## _t input, \
                           LPDRMETAENTRY entry, error_monitor_t em) {   \
    int r = 0;                                                          \
    if (input < __min) {                                                \
        CPE_ERROR(em, "write %s from %s, value(" __valuefmt ") "        \
                  "less than " __valuefmt,                              \
                  #__to, #__from, input, (__from ## _t)__min);          \
        r = -1;                                                         \
    }                                                                   \
    *((__to ## _t *)result)= (__to ## _t)input;                         \
    return r;                                                           \
}

#define DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(__to, __from, __min, __max, __valuefmt) \
int __to ## _from_ ## __from(void * result, __from ## _t input,                        \
                           LPDRMETAENTRY entry, error_monitor_t em) {   \
    int r = 0;                                                          \
    if (input < __min) {                                                \
        CPE_ERROR(em, "write %s from %s, value(" __valuefmt ") "        \
                  "less than " __valuefmt,                              \
                  #__to, #__from, input, (__from ## _t)__min);          \
        r = -1;                                                         \
    }                                                                   \
    if (input > __max) {                                                \
        CPE_ERROR(em, "write %s from %s, value(" __valuefmt ") "        \
                  "bigger than " __valuefmt,                            \
                  #__to, #__from, input, (__from ## _t)__max);          \
        r = -1;                                                         \
    }                                                                   \
    *(( __to ## _t *)result) = (__to ## _t)input;                       \
    return r;                                                           \
}

#define DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MAX(__to, __from, __max, __valuefmt)   \
int __to ## _from_ ## __from(void * result, __from ## _t input,                 \
                           LPDRMETAENTRY entry, error_monitor_t em) {   \
    int r = 0;                                                          \
    if (input > __max) {                                                \
        CPE_ERROR(em, "write %s from %s, value(" __valuefmt ") "        \
                  "bigger than " __valuefmt,                            \
                  #__to, #__from, input, (__from ## _t)__max);          \
        r = -1;                                                         \
    }                                                                   \
    *((__to ## _t *)result) = (__to ## _t)input;                        \
    return r;                                                           \
}

DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(int8, int8);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MAX(int8, uint8, SCHAR_MAX, "%u");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(int8, int16, SCHAR_MIN, SCHAR_MAX, "%d");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MAX(int8, uint16, SCHAR_MAX, "%u");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(int8, int32, SCHAR_MIN, SCHAR_MAX, "%d");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MAX(int8, uint32, SCHAR_MAX, "%u");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(int8, int64, SCHAR_MIN, SCHAR_MAX, "%lld");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MAX(int8, uint64, SCHAR_MAX, "%llu");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(int8, float, SCHAR_MIN, SCHAR_MAX, "%f");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(int8, double, SCHAR_MIN, SCHAR_MAX, "%f");

DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MIN(uint8, int8, 0, "%d");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(uint8, uint8);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(uint8, int16, 0, UCHAR_MAX, "%d");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MAX(uint8, uint16, UCHAR_MAX, "%u");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(uint8, int32, 0, UCHAR_MAX, "%d");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MAX(uint8, uint32, UCHAR_MAX, "%u");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(uint8, int64, 0, UCHAR_MAX, "%lld");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MAX(uint8, uint64, UCHAR_MAX, "%llu");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(uint8, float, 0, UCHAR_MAX, "%f");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(uint8, double, 0, UCHAR_MAX, "%f");

DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(int16, int8);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(int16, uint8);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(int16, int16);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MAX(int16, uint16, SHRT_MAX, "%u");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(int16, int32, SHRT_MIN, SHRT_MAX, "%d");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MAX(int16, uint32, SHRT_MAX, "%u");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(int16, int64, SHRT_MIN, SHRT_MAX, "%lld");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MAX(int16, uint64, SHRT_MAX, "%llu");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(int16, float, SHRT_MIN, SHRT_MAX, "%f");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(int16, double, SHRT_MIN, SHRT_MAX, "%f");

DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MIN(uint16, int8, 0, "%d");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(uint16, uint8);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MIN(uint16, int16, 0, "%d");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(uint16, uint16);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(uint16, int32, 0, USHRT_MAX, "%d");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MAX(uint16, uint32, USHRT_MAX, "%u");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(uint16, int64, 0, USHRT_MAX, "%lld");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MAX(uint16, uint64, USHRT_MAX, "%llu");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(uint16, float, 0, USHRT_MAX, "%f");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(uint16, double, 0, USHRT_MAX, "%f");

DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(int32, int8);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(int32, uint8);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(int32, int16);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(int32, uint16);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(int32, int32);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MAX(int32, uint32, INT_MAX, "%u");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(int32, int64, INT_MIN, INT_MAX, "%lld");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MAX(int32, uint64, INT_MAX, "%llu");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(int32, float, INT_MIN, INT_MAX, "%f");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(int32, double, INT_MIN, INT_MAX, "%f");

DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MIN(uint32, int8, 0, "%d");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(uint32, uint8);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MIN(uint32, int16, 0, "%d");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(uint32, uint16);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MIN(uint32, int32, 0, "%d");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(uint32, uint32);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(uint32, int64, 0, UINT_MAX, "%lld");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MAX(uint32, uint64, UINT_MAX, "%llu");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(uint32, float, 0, UINT_MAX, "%f");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(uint32, double, 0, UINT_MAX, "%f");

DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(int64, int8);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(int64, uint8);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(int64, int16);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(int64, uint16);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(int64, int32);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(int64, uint32);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(int64, int64);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(int64, uint64, 0, 9223372036854775807LL, "%llu");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(int64, float, -9223372036854775806LL, 9223372036854775807LL, "%f");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(int64, double, -9223372036854775806LL, 9223372036854775807LL, "%f");

DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MIN(uint64, int8, 0, "%d");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(uint64, uint8);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MIN(uint64, int16, 0, "%d");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(uint64, uint16);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MIN(uint64, int32, 0, "%d");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(uint64, uint32);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_MIN(uint64, int64, 0, "%lld");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(uint64, uint64);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(uint64, float, 0, 18446744073709551615ULL, "%f");
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_RANGE(uint64, double, 0, 18446744073709551615ULL, "%f");

DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(float, int8);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(float, uint8);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(float, int16);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(float, uint16);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(float, int32);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(float, uint32);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(float, int64);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(float, uint64);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(float, float);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(float, double);

DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(double, int8);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(double, uint8);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(double, int16);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(double, uint16);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(double, int32);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(double, uint32);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(double, int64);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(double, uint64);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(double, float);
DEF_WRITE_FUN_FROM_NUMERIC_CHECK_NONE(double, double);

int string_from_int8(void * result, int8_t input, LPDRMETAENTRY entry, error_monitor_t em) {
    return snprintf(result, entry->m_size, "%d", input) < entry->m_size ? 0 : -1;
}

int string_from_uint8(void * result, uint8_t input, LPDRMETAENTRY entry, error_monitor_t em) {
    return snprintf(result, entry->m_size, "%u", input) < entry->m_size ? 0 : -1;
}

int string_from_int16(void * result, int16_t input, LPDRMETAENTRY entry, error_monitor_t em) {
    return snprintf(result, entry->m_size, "%d", input) < entry->m_size ? 0 : -1;
}

int string_from_uint16(void * result, uint16_t input, LPDRMETAENTRY entry, error_monitor_t em) {
    return snprintf(result, entry->m_size, "%u", input) < entry->m_size ? 0 : -1;
}

int string_from_int32(void * result, int32_t input, LPDRMETAENTRY entry, error_monitor_t em) {
    return snprintf(result, entry->m_size, "%d", input) < entry->m_size ? 0 : -1;
}

int string_from_uint32(void * result, uint32_t input, LPDRMETAENTRY entry, error_monitor_t em) {
    return snprintf(result, entry->m_size, "%u", input) < entry->m_size ? 0 : -1;
}

int string_from_int64(void * result, int64_t input, LPDRMETAENTRY entry, error_monitor_t em) {
    return snprintf(result, entry->m_size, "%lld", input) < entry->m_size ? 0 : -1;
}

int string_from_uint64(void * result, uint64_t input, LPDRMETAENTRY entry, error_monitor_t em) {
    return snprintf(result, entry->m_size, "%llu", input) < entry->m_size ? 0 : -1;
}

int string_from_float(void * result, float input, LPDRMETAENTRY entry, error_monitor_t em) {
    return snprintf(result, entry->m_size, "%f", input) < entry->m_size ? 0 : -1;
}

int string_from_double(void * result, double input, LPDRMETAENTRY entry, error_monitor_t em) {
    return snprintf(result, entry->m_size, "%F", input) < entry->m_size ? 0 : -1;
}

struct DRCtypeTypeWriteOps g_dr_ctype_write_ops[] = {
     /*CPE_DR_TYPE_UNION*/
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    }
    , /*CPE_DR_TYPE_STRUCT*/
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    }
    , /*CPE_DR_TYPE_CHAR*/
    { int8_from_int8, int8_from_uint8
      , int8_from_int16, int8_from_uint16
      , int8_from_int32, int8_from_uint32
      , int8_from_int64, int8_from_uint64
      , int8_from_float, int8_from_double
    }
    , /*CPE_DR_TYPE_UCHAR*/
    { uint8_from_int8, uint8_from_uint8
      , uint8_from_int16, uint8_from_uint16
      , uint8_from_int32, uint8_from_uint32
      , uint8_from_int64, uint8_from_uint64
      , uint8_from_float, uint8_from_double
    }
    , /*CPE_DR_TYPE_INT8*/
    { int8_from_int8, int8_from_uint8
      , int8_from_int16, int8_from_uint16
      , int8_from_int32, int8_from_uint32
      , int8_from_int64, int8_from_uint64
      , int8_from_float, int8_from_double
    }
    , /*CPE_DR_TYPE_INT16*/
    { int16_from_int8, int16_from_uint8
      , int16_from_int16, int16_from_uint16
      , int16_from_int32, int16_from_uint32
      , int16_from_int64, int16_from_uint64
      , int16_from_float, int16_from_double
    }
    , /*CPE_DR_TYPE_UINT16*/
    { uint16_from_int8, uint16_from_uint8
      , uint16_from_int16, uint16_from_uint16
      , uint16_from_int32, uint16_from_uint32
      , uint16_from_int64, uint16_from_uint64
      , uint16_from_float, uint16_from_double
    }
    , /*CPE_DR_TYPE_INT32*/
    { int32_from_int8, int32_from_uint8
      , int32_from_int16, int32_from_uint16
      , int32_from_int32, int32_from_uint32
      , int32_from_int64, int32_from_uint64
      , int32_from_float, int32_from_double
    }
    , /*CPE_DR_TYPE_UINT32*/
    { uint32_from_int8, uint32_from_uint8
      , uint32_from_int16, uint32_from_uint16
      , uint32_from_int32, uint32_from_uint32
      , uint32_from_int64, uint32_from_uint64
      , uint32_from_float, uint32_from_double
    }
    , /*CPE_DR_TYPE_INT32*/
    { int32_from_int8, int32_from_uint8
      , int32_from_int16, int32_from_uint16
      , int32_from_int32, int32_from_uint32
      , int32_from_int64, int32_from_uint64
      , int32_from_float, int32_from_double
    }
    , /*CPE_DR_TYPE_UINT32*/
    { uint32_from_int8, uint32_from_uint8
      , uint32_from_int16, uint32_from_uint16
      , uint32_from_int32, uint32_from_uint32
      , uint32_from_int64, uint32_from_uint64
      , uint32_from_float, uint32_from_double
    }
    , /*CPE_DR_TYPE_INT64*/
    { int64_from_int8, int64_from_uint8
      , int64_from_int16, int64_from_uint16
      , int64_from_int32, int64_from_uint32
      , int64_from_int64, int64_from_uint64
      , int64_from_float, int64_from_double
    }
    , /*CPE_DR_TYPE_UINT64*/
    { uint64_from_int8, uint64_from_uint8
      , uint64_from_int16, uint64_from_uint16
      , uint64_from_int32, uint64_from_uint32
      , uint64_from_int64, uint64_from_uint64
      , uint64_from_float, uint64_from_double
    }
    , /*CPE_DR_TYPE_DATE*/
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    }
    , /*CPE_DR_TYPE_TIME*/
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    }
    , /*CPE_DR_TYPE_DATETIME*/
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    }
    , /*CPE_DR_TYPE_MONEY*/
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    }
    , /*CPE_DR_TYPE_FLOAT*/
    { float_from_int8, float_from_uint8
      , float_from_int16, float_from_uint16
      , float_from_int32, float_from_uint32
      , float_from_int64, float_from_uint64
      , float_from_float, float_from_double
    }
    , /*CPE_DR_TYPE_DOUBLE*/
    { double_from_int8, double_from_uint8
      , double_from_int16, double_from_uint16
      , double_from_int32, double_from_uint32
      , double_from_int64, double_from_uint64
      , double_from_float, double_from_double
    }
    , /*CPE_DR_TYPE_IP*/
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    }
    , /*CPE_DR_TYPE_WCHAR*/
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    }
    , /*CPE_DR_TYPE_STRING*/
    { string_from_int8, string_from_uint8
      , string_from_int16, string_from_uint16
      , string_from_int32, string_from_uint32
      , string_from_int64, string_from_uint64
      , string_from_float, string_from_double
    }
    , /*CPE_DR_TYPE_WSTRING*/
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    }
    , /*CPE_DR_TYPE_VOID*/
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    }
    , /*CPE_DR_TYPE_UINT8*/
    { uint8_from_int8, uint8_from_uint8
      , uint8_from_int16, uint8_from_uint16
      , uint8_from_int32, uint8_from_uint32
      , uint8_from_int64, uint8_from_uint64
      , uint8_from_float, uint8_from_double
    }
};

#define CPE_WRITEOPS_COUNT (sizeof(g_dr_ctype_write_ops) / sizeof(struct DRCtypeTypeWriteOps))

#define CPE_DEF_WRITE_FUN(__from)                                       \
    int dr_ctype_set_from_ ## __from(                                   \
        void * result,                                                  \
        __from ## _t input, int type, error_monitor_t em)               \
    {                                                                   \
        if (type < 0 || type > CPE_WRITEOPS_COUNT) {                    \
            CPE_ERROR(em, "write from %d, type is unknown", type);      \
            return -1;                                                  \
        }                                                               \
        else {                                                          \
            if (g_dr_ctype_write_ops[type].from_ ## __from) {           \
                return g_dr_ctype_write_ops[type].from_ ## __from (     \
                    result, input, NULL, em);                           \
            }                                                           \
            else {                                                      \
                CPE_ERROR(em, "write from %s, type not support to %d",  \
                          #__from, type);                               \
                return -1;                                              \
            }                                                           \
        }                                                               \
    }                                                                   \
    int dr_entry_set_from_ ## __from(                                   \
        void * result,                                                  \
        __from ## _t input, LPDRMETAENTRY entry, error_monitor_t em)    \
    {                                                                   \
        if (entry == NULL) {                                            \
            return -1;                                                  \
        }                                                               \
        if (entry->m_type < 0 || entry->m_type > CPE_WRITEOPS_COUNT) {  \
            CPE_ERROR(em, "write from %d, type is unknown",             \
                      entry->m_type);                                   \
            return -1;                                                  \
        }                                                               \
        else {                                                          \
            if (g_dr_ctype_write_ops[entry->m_type].from_ ## __from) {  \
                return g_dr_ctype_write_ops[entry->m_type].from_ ## __from ( \
                    result, input, entry, em);                          \
            }                                                           \
            else {                                                      \
                CPE_ERROR(em, "write from %s, type not support to %d",  \
                          #__from, entry->m_type);                      \
                return -1;                                              \
            }                                                           \
        }                                                               \
    }                                                                   \


CPE_DEF_WRITE_FUN(int8);
CPE_DEF_WRITE_FUN(uint8);
CPE_DEF_WRITE_FUN(int16);
CPE_DEF_WRITE_FUN(uint16);
CPE_DEF_WRITE_FUN(int32);
CPE_DEF_WRITE_FUN(uint32);
CPE_DEF_WRITE_FUN(int64);
CPE_DEF_WRITE_FUN(uint64);
CPE_DEF_WRITE_FUN(float);
CPE_DEF_WRITE_FUN(double);

