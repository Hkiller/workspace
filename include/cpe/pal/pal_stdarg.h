#ifndef CPE_PAL_STDARG_H
#define CPE_PAL_STDARG_H
#include <stdarg.h>

#if defined _MSC_VER
# define va_copy(t, s) (t) = (s)
#endif

#endif
