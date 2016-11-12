#ifndef CPE_PAL_STDIO_H
#define CPE_PAL_STDIO_H
#include <stdio.h>
#include "pal_types.h"

#ifdef __cplusplus
extern "C" {
#endif


#if defined _MSC_VER
#if _MSC_VER >= 1400  // VC 8.0 and later deprecate snprintf and _snprintf.
# ifdef snprintf
#   undef snprintf
# endif
# define snprintf(__buf, __size, __fmt, ...) _snprintf_s(__buf, __size,  _TRUNCATE, __fmt, __VA_ARGS__)
# define vsnprintf(__buf, __size, __fmt, __va) _vsnprintf_s(__buf, __size,  _TRUNCATE, __fmt, __va)
# define fileno _fileno
#else
# define snprintf _snprintf
#endif
#endif

#if defined _MSC_VER
# define FMT_SIZE_T "%u"
# define FMT_UINT64_T "%I64u"
# define FMT_INT64_T "%I64d"
# define FMT_UINT32_T "%u"
# define FMT_INT32_T "%d"
#else
# define FMT_SIZE_T "%zd"

# if (__WORDSIZE == 64)
#   if defined __llvm__ || defined __clang__
#     define FMT_UINT64_T "%llu"
#     define FMT_INT64_T "%lld"
#   else
#     define FMT_UINT64_T "%lu"
#     define FMT_INT64_T "%ld"
#   endif
#   define FMT_UINT32_T "%u"
#   define FMT_INT32_T "%d"
#   define FMT_UINT16_T "%hu"
#   define FMT_INT16_T "%hd"
# else
#   define FMT_UINT64_T "%llu"

#   define FMT_INT64_T "%lld"
#   define FMT_UINT32_T "%u"
#   define FMT_INT32_T "%d"
#   define FMT_UINT16_T "%hu"
#   define FMT_INT16_T "%hd"
# endif

#endif

#if (__WORDSIZE == 64)
#define FMT_PTR_INT_T FMT_INT64_T
#else
#define FMT_PTR_INT_T FMT_INT32_T
#endif

#ifdef __cplusplus
}
#endif

#endif
