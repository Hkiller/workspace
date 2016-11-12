/*****************************************************************************
 * dirent.h - dirent API for Microsoft Visual Studio
 *
 * Copyright (C) 2006 Toni Ronkko
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * ``Software''), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL TONI RONKKO BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Mar 15, 2011, Toni Ronkko
 * Defined FILE_ATTRIBUTE_DEVICE for MSVC 6.0.
 *
 * Aug 11, 2010, Toni Ronkko
 * Added d_type and d_namlen fields to dirent structure.  The former is
 * especially useful for determining whether directory entry represents a
 * file or a directory.  For more information, see
 * http://www.delorie.com/gnu/docs/glibc/libc_270.html
 *
 * Aug 11, 2010, Toni Ronkko
 * Improved conformance to the standards.  For example, errno is now set
 * properly on failure and assert() is never used.  Thanks to Peter Brockam
 * for suggestions.
 *
 * Aug 11, 2010, Toni Ronkko
 * Fixed a bug in rewinddir(): when using relative directory names, change
 * of working directory no longer causes rewinddir() to fail.
 *
 * Dec 15, 2009, John Cunningham
 * Added rewinddir member function
 *
 * Jan 18, 2008, Toni Ronkko
 * Using FindFirstFileA and WIN32_FIND_DATAA to avoid converting string
 * between multi-byte and unicode representations.  This makes the
 * code simpler and also allows the code to be compiled under MingW.  Thanks
 * to Azriel Fasten for the suggestion.
 *
 * Mar 4, 2007, Toni Ronkko
 * Bug fix: due to the strncpy_s() function this file only compiled in
 * Visual Studio 2005.  Using the new string functions only when the
 * compiler version allows.
 *
 * Nov  2, 2006, Toni Ronkko
 * Major update: removed support for Watcom C, MS-DOS and Turbo C to
 * simplify the file, updated the code to compile cleanly on Visual
 * Studio 2005 with both unicode and multi-byte character strings,
 * removed rewinddir() as it had a bug.
 *
 * Aug 20, 2006, Toni Ronkko
 * Removed all remarks about MSVC 1.0, which is antiqued now.  Simplified
 * comments by removing SGML tags.
 *
 * May 14 2002, Toni Ronkko
 * Embedded the function definitions directly to the header so that no
 * source modules need to be included in the Visual Studio project.  Removed
 * all the dependencies to other projects so that this very header can be
 * used independently.
 *
 * May 28 1998, Toni Ronkko
 * First version.
 *****************************************************************************/
#ifndef DIRENT_H
#define DIRENT_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

/* Entries missing from MSVC 6.0 */
#if !defined(FILE_ATTRIBUTE_DEVICE)
# define FILE_ATTRIBUTE_DEVICE 0x40
#endif

/* File type and permission flags for stat() */
#if defined(_MSC_VER)  &&  !defined(S_IREAD)
# define S_IFMT   _S_IFMT                      /* file type mask */
# define S_IFDIR  _S_IFDIR                     /* directory */
# define S_IFCHR  _S_IFCHR                     /* character device */
# define S_IFFIFO _S_IFFIFO                    /* pipe */
# define S_IFREG  _S_IFREG                     /* regular file */
# define S_IREAD  _S_IREAD                     /* read permission */
# define S_IWRITE _S_IWRITE                    /* write permission */
# define S_IEXEC  _S_IEXEC                     /* execute permission */
#endif

#if !defined(S_IFBLK)
#define S_IFBLK   0                            /* block device */
#define S_IFLNK   0                            /* link */
#define S_IFSOCK  0                            /* socket */
#endif

#if defined(_WIN32)
# define S_IRUSR  S_IREAD                      /* read, user */
# define S_IWUSR  S_IWRITE                     /* write, user */
# define S_IXUSR  0                            /* execute, user */
# define S_IRGRP  0                            /* read, group */
# define S_IWGRP  0                            /* write, group */
# define S_IXGRP  0                            /* execute, group */
# define S_IROTH  0                            /* read, others */
# define S_IWOTH  0                            /* write, others */
# define S_IXOTH  0                            /* execute, others */
#endif

#if ! defined S_IRWXU
# define S_IRWXU ( S_IRUSR | S_IWUSR | S_IXUSR )
#endif

/* Indicates that d_type field is available in dirent structure */
#define _DIRENT_HAVE_D_TYPE

/* File type flags for d_type */
#define DT_UNKNOWN  0
#define DT_REG      S_IFREG
#define DT_DIR      S_IFDIR
#define DT_FIFO     S_IFFIFO
#define DT_SOCK     S_IFSOCK
#define DT_CHR      S_IFCHR
#define DT_BLK      S_IFBLK

/* Macros for converting between st_mode and d_type */
#define IFTODT(mode) ((mode) & S_IFMT)
#define DTTOIF(type) (type)

/*
 * File type macros.  Note that block devices, sockets and links cannot be
 * distinguished on Windows and the macros S_ISBLK, S_ISSOCK and S_ISLNK are
 * only defined for compatibility.  These macros should always return false
 * on Windows.
 */
#if !defined(S_ISFIFO)
#define	S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFFIFO)
#define	S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#define	S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#define	S_ISLNK(mode)  (((mode) & S_IFMT) == S_IFLNK)
#define	S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)
#define	S_ISCHR(mode)  (((mode) & S_IFMT) == S_IFCHR)
#define	S_ISBLK(mode)  (((mode) & S_IFMT) == S_IFBLK)
#endif

#ifdef __cplusplus
extern "C" {
#endif


typedef struct dirent
{
   char d_name[MAX_PATH + 1];                  /* File name */
   size_t d_namlen;                            /* Length of name without \0 */
   int d_type;                                 /* File type */
} dirent;


typedef struct DIR
{
   dirent           curentry;                  /* Current directory entry */
   WIN32_FIND_DATAA find_data;                 /* Private file data */
   int              cached;                    /* True if data is valid */
   HANDLE           search_handle;             /* Win32 search handle */
   char             patt[MAX_PATH + 3];        /* Initial directory name */
} DIR;


/* Forward declarations */
DIR *opendir(const char *dirname);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
void rewinddir(DIR* dirp);

#ifdef __cplusplus
}
#endif
#endif /*DIRENT_H*/
