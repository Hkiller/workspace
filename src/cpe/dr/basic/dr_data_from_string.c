#define _ISOC99_SOURCE
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"

#if defined _MSC_VER
#    pragma warning(disable: 4018)
#endif

struct DRCtypeTypeFromStringOps {
    int (* set_from_string)(void * output, LPDRMETAENTRY entry, const char * input, error_monitor_t em);
};

static int dr_set_string_from_string(void * output, LPDRMETAENTRY entry, const char * input, error_monitor_t em) {
    size_t len;

    assert(entry);

    len = strlen(input);
    if (len + 1 > (size_t)entry->m_size) {
        len = entry->m_size - 1;
    }

    memcpy(output, input, len);
    *((char *)output + len) = 0;
    return 0;
}

#define DR_TYPE_BUILD_READ_INT_FUN(__bit, __min, __max)                 \
static int dr_set_int ## __bit ## _from_string(void * output, LPDRMETAENTRY entry, const char * s, error_monitor_t em) { \
    int ## __bit ## _t acc;                                                        \
    int base;                                                           \
    char c;                                                             \
    uint ## __bit ## _t cutoff;                                         \
    int ## __bit ## _t neg, any, cutlim;                                \
                                                                        \
    do {                                                                \
        c = *s++;                                                       \
    } while (isspace((unsigned char)c));                                \
                                                                        \
    if (c != '-' && c != '+' && (c < '0' || c > '9')) {                 \
        if (entry) {                                                    \
            LPDRMETALIB metalib = dr_meta_owner_lib(                    \
                dr_entry_self_meta(entry));                             \
            LPDRMACRO macro = dr_lib_macro_find(metalib, s - 1);        \
            if (macro == NULL) return -1;                               \
            *((int ## __bit ## _t*)output) = dr_macro_value(macro);     \
            return 0;                                                   \
        }                                                               \
        else {                                                          \
            return -1;                                                  \
        }                                                               \
    }                                                                   \
                                                                        \
    if (c == '-') {                                                     \
        neg = 1;                                                        \
        c = *s++;                                                       \
    }                                                                   \
    else {                                                              \
    neg = 0;                                                            \
    if (c == '+') {                                                     \
        c = *s++;                                                       \
    }                                                                   \
    }                                                                   \
                                                                        \
    base = 10;                                                          \
    if (c == '0' && (*s == 'x' || *s == 'X') &&                         \
                 ((s[1] >= '0' && s[1] <= '9') ||                       \
                  (s[1] >= 'A' && s[1] <= 'F') ||                       \
                  (s[1] >= 'a' && s[1] <= 'f')))                        \
    {                                                                   \
    c = s[1];                                                           \
    s += 2;                                                             \
    base = 16;                                                          \
    }                                                                   \
                                                                        \
    acc = any = 0;                                                      \
                                                                        \
    cutoff = neg                                                        \
        ? (uint ## __bit ## _t)-(__min + __max) + __max                 \
        : __max;                                                        \
    cutlim = cutoff % base;                                             \
    cutoff /= base;                                                     \
                                                                        \
    for ( ; ; c = *s++) {                                               \
    if (c >= '0' && c <= '9')                                           \
        c -= '0';                                                       \
    else if (c >= 'A' && c <= 'Z')                                      \
        c -= 'A' - 10;                                                  \
    else if (c >= 'a' && c <= 'z')                                      \
        c -= 'a' - 10;                                                  \
    else                                                                \
        break;                                                          \
                                                                        \
    if (c >= base) {                                                    \
        break;                                                          \
    }                                                                   \
                                                                        \
    if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {     \
        any = -1;                                                       \
    }                                                                   \
    else {                                                              \
    any = 1;                                                            \
    acc *= base;                                                        \
    acc += c;                                                           \
    }                                                                   \
    }                                                                   \
                                                                        \
    if (any < 0) { /*overflow*/                                         \
        return -1;                                                      \
    } else if (!any) { /*no data*/                                      \
        return -1;                                                      \
    }                                                                   \
                                                                        \
    if (neg) {                                                          \
        acc = -acc;                                                     \
    }                                                                   \
                                                                        \
    *((int ## __bit ## _t*)output) = acc;                               \
                                                                        \
    return *(s - 1) == 0 ? 0 : -1;                                      \
}

#define DR_TYPE_BUILD_READ_UINT_FUN(__bit, __max)                       \
static int dr_set_uint ## __bit ## _from_string(                        \
    void * output, LPDRMETAENTRY entry,                                 \
    const char * s, error_monitor_t em)                                 \
{                                                                       \
    uint ## __bit ## _t acc;                                            \
    int base;                                                           \
    char c;                                                             \
    uint ## __bit ##_t cutoff;                                          \
    int ## __bit ## _t any, cutlim;                                     \
                                                                        \
    do {                                                                \
        c = *s++;                                                       \
    } while (isspace((unsigned char)c));                                \
                                                                        \
    if (c != '-' && c != '+' && (c < '0' || c > '9')) {                 \
        if (entry) {                                                    \
            LPDRMETALIB metalib = dr_meta_owner_lib(                    \
                dr_entry_self_meta(entry));                             \
            LPDRMACRO macro = dr_lib_macro_find(metalib, s - 1);        \
            if (macro == NULL) return -1;                               \
            *((uint ## __bit ## _t*)output) = dr_macro_value(macro);    \
            return 0;                                                   \
        }                                                               \
        else {                                                          \
            return -1;                                                  \
        }                                                               \
    }                                                                   \
                                                                        \
    if (c == '-') {                                                     \
        return -1;                                                      \
    }                                                                   \
                                                                        \
    if (c == '+') {                                                     \
        c = *s++;                                                       \
    }                                                                   \
                                                                        \
    base = 10;                                                          \
    if (c == '0' && (*s == 'x' || *s == 'X') &&                         \
        ((s[1] >= '0' && s[1] <= '9') ||                                \
         (s[1] >= 'A' && s[1] <= 'F') ||                                \
         (s[1] >= 'a' && s[1] <= 'f')))                                 \
    {                                                                   \
        c = s[1];                                                       \
        s += 2;                                                         \
        base = 16;                                                      \
    }                                                                   \
                                                                        \
    any = 0;                                                            \
    acc = (uint ## __bit ##_t) 0;                                       \
                                                                        \
    cutoff = __max;                                                     \
    cutlim = cutoff % base;                                             \
    cutoff /= base;                                                     \
                                                                        \
    for ( ; ; c = *s++) {                                               \
        if (c >= '0' && c <= '9')                                       \
            c -= '0';                                                   \
        else if (c >= 'A' && c <= 'Z')                                  \
            c -= 'A' - 10;                                              \
        else if (c >= 'a' && c <= 'z')                                  \
            c -= 'a' - 10;                                              \
        else                                                            \
            break;                                                      \
                                                                        \
        if (c >= base) {                                                \
            break;                                                      \
        }                                                               \
                                                                        \
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) { \
            any = -1;                                                   \
        }                                                               \
        else {                                                          \
            any = 1;                                                    \
            acc *= base;                                                \
            acc += c;                                                   \
        }                                                               \
    }                                                                   \
                                                                        \
    if (any < 0) { /*overflow*/                                         \
        return -1;                                                      \
    } else if (!any) { /*no data*/                                      \
        return -1;                                                      \
    }                                                                   \
                                                                        \
    *((uint ## __bit ## _t*)output) = acc;                              \
                                                                        \
    return *(s - 1) == 0 ? 0 : -1;                                      \
}

#ifndef INT64_MIN
#define INT64_MIN _I64_MIN
#endif

#ifndef INT64_MAX
#define INT64_MAX _I64_MAX
#endif

#ifndef UINT64_MAX
#define UINT64_MAX _UI64_MAX
#endif

DR_TYPE_BUILD_READ_INT_FUN(8, CHAR_MIN, CHAR_MAX)
DR_TYPE_BUILD_READ_UINT_FUN(8, UCHAR_MAX)
DR_TYPE_BUILD_READ_INT_FUN(16, SHRT_MIN, SHRT_MAX)
DR_TYPE_BUILD_READ_UINT_FUN(16, USHRT_MAX)
DR_TYPE_BUILD_READ_INT_FUN(32, INT_MIN, INT_MAX)
DR_TYPE_BUILD_READ_UINT_FUN(32, UINT_MAX)
DR_TYPE_BUILD_READ_INT_FUN(64, INT64_MIN, INT64_MAX)
DR_TYPE_BUILD_READ_UINT_FUN(64, UINT64_MAX)

#ifdef _CPE_NO_STRTOF
static int dr_set_float_from_string(void * output, LPDRMETAENTRY entry, const char * s, error_monitor_t em) {
    char * end;
    double b;

    assert(output);
    
    end = NULL;
    b = strtod(s, &end);

    if (*end != 0) {
        CPE_ERROR(em, "scan float from string %s fail, end point is %d", s, (end - s));
        return -1;
    }
    else if (b < -3.40E+38 || b > +3.40E+38 ) {
        CPE_ERROR(em, "scan float from string %s fail, value %f overflow", s, b);
        return -1;
    }
    else {
        *((float*)output) = (float)b;
        return 0;
    }
}
#else
static int dr_set_float_from_string(void * output, LPDRMETAENTRY entry, const char * s, error_monitor_t em) { 
    char * end;
    float b;

    assert(output);
    
    end = NULL;
    b = strtof(s, &end);

    if (*end != 0) {
        CPE_ERROR(em, "scan float from string %s fail, end point is %d", s, (int)(end - s));
        return -1;
    }
    else {
        *((float*)output) = b;
        return 0;
    }
}
#endif

static int dr_set_double_from_string(void * output, LPDRMETAENTRY entry, const char * s, error_monitor_t em) { 
    char * end;
    double b;

    assert(output);
    
    end = NULL;
    b = strtod(s, &end);

    if (*end != 0) {
        CPE_ERROR(em, "scan double from string %s fail, end point is %d", s, (int)(end - s));
        return -1;
    }
    else {
        *((double*)output) = b;
        return 0;
    }
}

struct DRCtypeTypeFromStringOps g_dr_from_string_ops[] = {
    {/*CPE_DR_TYPE_UNION*/ NULL}
    , {/*CPE_DR_TYPE_STRUCT*/ NULL }
    , {/*CPE_DR_TYPE_CHAR*/ dr_set_int8_from_string}
    , {/*CPE_DR_TYPE_UCHAR*/ dr_set_uint8_from_string}
    , {/*CPE_DR_TYPE_INT8*/ dr_set_int8_from_string}
    , {/*CPE_DR_TYPE_INT16*/ dr_set_int16_from_string}
    , {/*CPE_DR_TYPE_UINT16*/ dr_set_uint16_from_string}
    , {/*CPE_DR_TYPE_INT32*/ dr_set_int32_from_string}
    , {/*CPE_DR_TYPE_UINT32*/ dr_set_uint32_from_string}
    , {/*CPE_DR_TYPE_INT32*/ dr_set_int32_from_string}
    , {/*CPE_DR_TYPE_UINT32*/ dr_set_uint32_from_string}
    , {/*CPE_DR_TYPE_INT64*/ dr_set_int64_from_string}
    , {/*CPE_DR_TYPE_UINT64*/ dr_set_uint64_from_string}
    , {/*CPE_DR_TYPE_DATE*/ NULL}
    , {/*CPE_DR_TYPE_TIME*/ NULL}
    , {/*CPE_DR_TYPE_DATETIME*/ NULL}
    , {/*CPE_DR_TYPE_MONEY*/ NULL}
    , {/*CPE_DR_TYPE_FLOAT*/ dr_set_float_from_string}
    , {/*CPE_DR_TYPE_DOUBLE*/ dr_set_double_from_string}
    , {/*CPE_DR_TYPE_IP*/ NULL}
    , {/*CPE_DR_TYPE_WCHAR*/ NULL}
    , {/*CPE_DR_TYPE_STRING*/ dr_set_string_from_string}
    , {/*CPE_DR_TYPE_STRING*/ dr_set_string_from_string}
    , {/*CPE_DR_TYPE_VOID*/ NULL}
    , {/*CPE_DR_TYPE_UINT8*/ dr_set_uint8_from_string}
};

int dr_entry_set_from_string(void * output, const char * input, LPDRMETAENTRY entry, error_monitor_t em) {
    assert(entry);

    if (entry->m_type < 0 || entry->m_type > sizeof(g_dr_from_string_ops) / sizeof(struct DRCtypeTypeFromStringOps) ) {
        CPE_ERROR(em, "set %d from string, type is unknown", entry->m_type);
        return -1;
    }

    if (g_dr_from_string_ops[entry->m_type].set_from_string) {
        return g_dr_from_string_ops[entry->m_type].set_from_string(output, entry, input, em);
    }
    else {
        CPE_ERROR(em, "set %d from string, type not support", entry->m_type);
        return -1;
    }
}

int dr_ctype_set_from_string(void * output, int type, const char * input, error_monitor_t em) {
    if (type < 0 || type > sizeof(g_dr_from_string_ops) / sizeof(struct DRCtypeTypeFromStringOps) ) {
        CPE_ERROR(em, "set %d from string, type is unknown", type);
        return -1;
    }

    if (g_dr_from_string_ops[type].set_from_string) {
        return g_dr_from_string_ops[type].set_from_string(output, NULL, input, em);
    }
    else {
        return -1;
    }
}
