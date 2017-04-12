#ifndef CPE_UTILS_ASSERT_H
#define CPE_UTILS_ASSERT_H
#include <assert.h>
#include "cpe/pal/pal_stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void __cpe_assert_msg(const char *condition, const char *file, int line, int isError, int isHardError, const char *message, ...);

#if ! DEBUG
	#define	cpe_assert_warn(__condition__, ...)
#else
	#define cpe_assert_warn(__condition__, ...) if(!(__condition__)) __cpe_assert_msg(#__condition__, __FILE__, __LINE__, 0, 0, __VA_ARGS__)
#endif

#if ! DEBUG
	#define	cpe_assert_soft(__condition__, ...)
#else
	#define cpe_assert_soft(__condition__, ...) if(!(__condition__)){__cpe_assert_msg(#__condition__, __FILE__, __LINE__, 1, 0, __VA_ARGS__), abort();}
#endif

// Hard assertions are used in situations where the program definitely will crash anyway, and the reason is inexpensive to detect.
#define cpe_assert_hard(__condition__, ...) if(!(__condition__)){__cpe_assert_msg(#__condition__, __FILE__, __LINE__, 1, 1, __VA_ARGS__); abort();}

#ifdef __cplusplus
}
#endif

#endif
