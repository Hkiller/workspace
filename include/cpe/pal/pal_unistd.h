#ifndef CPE_PAL_UNISTD_H
#define CPE_PAL_UNISTD_H

#if defined _WIN32
#include <io.h>
#include <process.h>
#include <direct.h>
#define mkdir(__f, __m) _mkdir(__f)
#define rmdir _rmdir
#define unlink _unlink
#define getcwd _getcwd
#define chdir _chdir
#define fileno _fileno
#else
#include <unistd.h>
#endif

#endif
