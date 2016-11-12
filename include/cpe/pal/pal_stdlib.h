#ifndef CPE_PAL_STDLIB_H
#define CPE_PAL_STDLIB_H
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#define _CPE_NO_STRTOF
#endif

#ifdef _CPE_NO_STRTOF
float strtof(const char * s, char const ** endptr);
#endif

#ifdef _MSC_VER  
#define strtoll _strtoi64   
#define strtoull _strtoui64   
#endif

#ifdef __cplusplus
}
#endif

#endif
