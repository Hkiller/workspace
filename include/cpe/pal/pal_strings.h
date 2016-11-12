#ifndef CPE_PAL_STRINGS_H
#define CPE_PAL_STRINGS_H

#if defined _WIN32
#include <string.h>
#else
#include <strings.h>
#endif

#if defined _WIN32 || defined EMSCRIPTEN
# define bzero(p,l) memset((p),0,(l))
#endif

#endif
