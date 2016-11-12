#ifndef CPE_PAL_STRING_H
#define CPE_PAL_STRING_H

#include <string.h>

#if defined _MSC_VER
#define strdup _strdup
#define strcasecmp stricmp
#if ! defined strncasecmp
#    define strncasecmp strnicmp 
#endif
#define strnstr cpe_strnstr 
#pragma warning(disable:4996)
#endif

#if defined __CYGWIN__
#define strnstr cpe_strnstr 
#endif

#ifdef __cplusplus
extern "C" {
#endif

char * cpe_strnstr(const char *phaystack, const char *pneedle, const int phaystack_len);

#ifdef __cplusplus
}
#endif

#endif
