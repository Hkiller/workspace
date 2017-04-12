#ifndef CPE_PAL_FCNTL_H
#define CPE_PAL_FCNTL_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined _WIN32 /*windows*/
#include <windows.h>
#include <fcntl.h> 
#include <errno.h>

#else
/*posix */

#include <fcntl.h> 
#include <errno.h>

#if !defined(O_BINARY)
#  define O_BINARY 0
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif

