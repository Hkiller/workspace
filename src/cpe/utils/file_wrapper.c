#include <sys/stat.h>
#include <errno.h>
#include "cpe/pal/pal_unistd.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_dirent.h"
#include "cpe/pal/pal_string.h"
#include "file_internal.h"

int dir_mk(const char * path, mode_t mode, error_monitor_t em) {
    int rv;

    rv = mkdir(path, mode);

    if (rv != 0) {
        switch(errno) {
        case EACCES:
            CPE_ERROR_EX(
                em, errno,
                "Search permission is denied on a component of the path prefix,"
                " or write permission is denied on the parent directory of the directory to be created.");
            break;
        case EEXIST:
            break;
#if defined ELOOP
        case ELOOP:
            CPE_ERROR_EX(
                em, errno,
                "A loop exists in symbolic links encountered during resolution of the path argument.");
            break;
#endif
        case EMLINK:
            CPE_ERROR_EX(
                em, errno,
                "The link count of the parent directory would exceed {LINK_MAX}.");
            break;
        case ENAMETOOLONG:
            CPE_ERROR_EX(
                em, errno,
                "The length of the path argument exceeds {PATH_MAX} or a pathname component is longer than {NAME_MAX}.");
            break;
        case ENOENT:
            CPE_ERROR_EX(
                em, errno,
                "A component of the path prefix specified by path does not name an existing directory or path is an empty string.");
            break;
        case ENOSPC:
            CPE_ERROR_EX(
                em, errno,
                "The file system does not contain enough space to hold the contents of the new directory"
                " or to extend the parent directory of the new directory.");
            break;
        case ENOTDIR:
            CPE_ERROR_EX(
                em, errno,
                "A component of the path prefix is not a directory.");
        case EROFS:
            CPE_ERROR_EX(
                em, errno,
                "The parent directory resides on a read-only file system.");
            break;
        default:
            CPE_ERROR_EX(em, errno, "mmkdir unknown error, errno=%d.", errno);
            break;
        }
    }

    return rv;
}

DIR * dir_open(const char * path, int ignoreError, error_monitor_t em) {
    DIR * dp = opendir(path);
    if (dp == NULL) {
        if (errno == ignoreError) return dp;

        switch(errno) {
        case EACCES:
            CPE_ERROR_EX(em, errno, "open dir %s: Permission denied.", path);
            break;
        case EMFILE:
            CPE_ERROR_EX(em, errno, "open dir %s: Too many file descriptors in use by process.", path);
            break;
        case ENFILE:
            CPE_ERROR_EX(em, errno, "open dir %s: Too many files are currently open in the system.", path);
            break;
        case ENOENT:
            CPE_ERROR_EX(em, errno, "open dir %s: not exist.", path);
            break;
        case ENOMEM:
            CPE_ERROR_EX(em, errno, "open dir %s: Insufficient memory to complete the operation.", path);
            break;
        default:
            CPE_ERROR_EX(em, errno, "open dir %s: opendir unknown error.", path);
            break;
        }
    }
    return dp;
}

void dir_close(DIR * dirp, error_monitor_t em) {
    if (closedir(dirp) != 0) {
        switch(errno) {
        case EBADF:
            CPE_ERROR_EX(em, errno, "Invalid directory stream descriptor dir.");
            break;
        default:
            CPE_ERROR_EX(em, errno, "closedir unknown error.");
            break;
        }
    }
}

int dir_rm(const char *path, error_monitor_t em) {
    int rv;
    rv = rmdir(path);

    if (rv != 0) {
        switch(errno) {
        case EACCES:
            CPE_ERROR_EX(
                em, errno, 
                "Search permission is denied on a component of the path prefix, or write permission is denied"
                " on the parent directory of the directory to be removed.");
            break;
        case EBUSY:
            CPE_ERROR_EX(
                em, errno, 
                "The directory to be removed is currently in use by the system or some process"
                " and the implementation considers this to be an error.");
            break;
        case EEXIST:
        case ENOTEMPTY:
            CPE_ERROR_EX(
                em, errno, 
                "The path argument names a directory that is not an empty directory,"
                " or there are hard links to the directory other than dot or a single entry in dot-dot.");
            break;
        case EINVAL:
            CPE_ERROR_EX(
                em, errno, 
                "The path argument contains a last component that is dot.");
            break;
        case EIO:
            CPE_ERROR_EX(
                em, errno, 
                "A physical I/O error has occurred.");
            break;
#if defined ELOOP
        case ELOOP:
            CPE_ERROR_EX(
                em, errno, 
                "A loop exists in symbolic links encountered during resolution of the path argument.");
            break;
#endif
        case ENAMETOOLONG:
            CPE_ERROR_EX(
                em, errno, 
                "The length of the path argument exceeds {PATH_MAX} or a pathname component is longer than {NAME_MAX}.");
        case ENOENT:
            CPE_ERROR_EX(
                em, errno, 
                "ENOENT A component of path does not name an existing file, or the path argument names a"
                " nonexistent directory or points to an empty string.");
            break;
        case ENOTDIR:
            CPE_ERROR_EX(
                em, errno, 
                "A component of path is not a directory.");
            break;
        case EPERM:
            CPE_ERROR_EX(
                em, errno, 
                "The  S_ISVTX  flag  is  set  on  the parent directory of the directory to be removed"
                " and the caller is not the owner of the directory to be removed, nor is the"
                " caller the owner of the parent directory, nor does the caller have the appropriate privileges.");
            break;
        case EROFS:
            CPE_ERROR_EX(
                em, errno, 
                "The directory entry to be removed resides on a read-only file system.");
        }
    }
    return rv;
}

int file_rm(const char *path, error_monitor_t em) {
    int rv;
    rv = unlink(path);
    if (rv != 0) {
        switch(errno) {
        case EACCES:
            CPE_ERROR_EX(
                em, errno,
                "Search permission is denied for a component  of  the  path  prefix,"
                "  or write  permission  is  denied on the directory containing"
                " the directory entry to be removed.");
            break;
        case EBUSY:
            CPE_ERROR_EX(
                em, errno,
                "The file named by the path argument cannot be unlinked  because  it"
                "  is being used by the system or another process and the implementation"
                " considers this an error.");
            break;
#if defined ELOOP
        case ELOOP:
            CPE_ERROR_EX(
                em, errno,
                "A loop exists in symbolic links encountered during  resolution  of"
                " the path argument.");
            break;
#endif
        case ENAMETOOLONG:
            CPE_ERROR_EX(
                em, errno,
                "The length of the path argument exceeds {PATH_MAX} or a pathname"
                " component is longer than {NAME_MAX}.");
            break;
        case ENOENT:
            CPE_ERROR_EX(
                em, errno,
                "A component of path does not name an existing file or path is an empty string.");
            break;
        case ENOTDIR:
            CPE_ERROR_EX(
                em, errno,
                "A component of the path prefix is not a directory.");
            break;
        case EPERM:
            CPE_ERROR_EX(
                em, errno,
                "The  file  named by path is a directory, and either the calling process"
                " does not have appropriate privileges, or the  implementation  prohibits"
                " using unlink() on directories.");
            break;
        case EROFS:
            CPE_ERROR_EX(
                em, errno,
                "The directory entry to be unlinked is part of a read-only file  system.");
            break;
        default:
            CPE_ERROR_EX(
                em, errno,
                "unlink return unknown error, errno=%d.", errno);
            break;
        }
    }

    return rv;
}

FILE * file_stream_open(const char *path, const char *mode, error_monitor_t em) {
    FILE * fp = fopen(path, mode);
    if (fp == NULL) {
        CPE_ERROR(em, "open file %s fail, errno=%d (%s)!", path, errno, strerror(errno));
    }
    return fp;
}

void file_stream_close(FILE * fp, error_monitor_t em) {
    int rv = fclose(fp);
    if (rv != 0) {
        CPE_ERROR(em, "close file fail, errno=%d!", errno);
    }
}

int inode_stat_by_path(const char * path, struct stat * buf, int ignoreError, error_monitor_t em) {
    int status;
    status = stat(path, buf);
    if (status == -1) {
        if (errno == ignoreError) return status;

        switch(errno) {
        case EACCES:
            CPE_ERROR_EX(em, errno, "Search permission is denied for a component of the path prefix.");
            break;
        case EIO:
            CPE_ERROR_EX(em, errno, "An error occurred while reading from the file system.");
            break;
#if defined ELOOP
        case ELOOP:
            CPE_ERROR_EX(em, errno, "A loop exists in symbolic links encountered during resolution of the path argument.");
            break;
#endif
        case ENAMETOOLONG:
            CPE_ERROR_EX(em, errno, "The length of the path argument exceeds {PATH_MAX} or a pathname component is longer than {NAME_MAX}.");
            break;
        case ENOENT:
            CPE_ERROR_EX(em, errno, "A component of path does not name an existing file or path is an empty string.");
            break;
        case ENOTDIR:
            CPE_ERROR_EX(em, errno, "A component of the path prefix is not a directory.");
            break;
#if defined EOVERFLOW
        case EOVERFLOW:
            CPE_ERROR_EX(
                em, errno,
                "The file size in bytes or the number of blocks allocated to the file "
                "or the file serial number cannot be represented correctly in the structure "
                "pointed  to  by buf.");
            break;
#endif
        }
    }
    return status;
}

int inode_stat_by_fileno(int fno, struct stat * buf, int ignoreError, error_monitor_t em) {
    int status;
    status = fstat(fno, buf);
    if (status == 0) {
        if (errno == ignoreError) return status;

        switch(errno) {
        case EACCES:
            CPE_ERROR_EX(em, errno, "Search permission is denied for a component of the path prefix.");
            break;
        case EIO:
            CPE_ERROR_EX(em, errno, "An error occurred while reading from the file system.");
            break;
#if defined ELOOP
        case ELOOP:
            CPE_ERROR_EX(em, errno, "A loop exists in symbolic links encountered during resolution of the path argument.");
            break;
#endif
        case ENAMETOOLONG:
            CPE_ERROR_EX(em, errno, "The length of the path argument exceeds {PATH_MAX} or a pathname component is longer than {NAME_MAX}.");
            break;
        case ENOTDIR:
            CPE_ERROR_EX(em, errno, "A component of the path prefix is not a directory.");
            break;
#if defined EOVERFLOW
        case EOVERFLOW:
            CPE_ERROR_EX(
                em, errno,
                "The file size in bytes or the number of blocks allocated to the file "
                "or the file serial number cannot be represented correctly in the structure "
                "pointed  to  by buf.");
            break;
#endif
        }
    }
    return status;
}

int DIR_DEFAULT_MODE = S_IRWXU | S_IRGRP | S_IXGRP;
int FILE_DEFAULT_MODE = S_IRUSR | S_IWUSR | S_IRGRP;
