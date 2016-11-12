#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "file_internal.h"

#ifndef MAXNAMLEN
#	define MAXNAMLEN 256
#endif

int dir_mk_recursion(const char * path, mode_t mode, error_monitor_t em, mem_allocrator_t talloc) {
    size_t path_len = strlen(path) + 1;
    char * path_buf = (char *)mem_alloc(talloc, path_len);
    char * nextSepPos;
    int rv = 0;

    memcpy(path_buf, path, path_len);

    for(nextSepPos = strchr(path_buf, '/');
        nextSepPos && rv == 0;
        nextSepPos = strchr(nextSepPos + 1, '/'))
    {
        if (nextSepPos == path_buf) continue; /*ignore root dir*/

        *nextSepPos = 0;

#ifdef __CYGWIN__
        if (!dir_exist(path_buf, em)) {
            rv = dir_mk(path_buf, mode, em);
        }
#else
        rv = dir_mk(path_buf, mode, em);
        if (rv && errno == EEXIST) rv = 0;
#endif
        *nextSepPos = '/';
    }

    if (rv == 0) {
#ifdef __CYGWIN__
        if (!dir_exist(path_buf, em)) {
            rv = dir_mk(path_buf, mode, em);
        }
#else
        rv = dir_mk(path_buf, mode, em);
        if (rv && errno == EEXIST) rv = 0;
#endif
    }

    mem_free(talloc, path_buf);
    return rv;
}

int dir_rm_recursion(const char * path, error_monitor_t em, mem_allocrator_t talloc) {
    DIR * dirp;
    struct dirent dbuf;
    struct dirent * dp;
    int rv = 0;

    char * subPath = NULL;
    size_t pathSize;

    dirp = dir_open(path, ENOENT, em);
    if (dirp == NULL) {
        return errno == ENOENT ? 0 : -1;
    }

    pathSize = strlen(path);

    (void)dbuf;
    /*remove sub entities*/
    while((rv = readdir_r(dirp, &dbuf, &dp)) == 0 && dp) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;

        if (subPath == NULL) {
            subPath = (char *)mem_alloc(talloc, pathSize + 1/*path sep*/ + MAXNAMLEN + 1);
            if (subPath == NULL) {
                CPE_ERROR_EX(em, ENOMEM, "alloc for path buf fail!");
                rv = -1;
                break;
            }

            memcpy(subPath, path, pathSize);
            subPath[pathSize] = '/';
        }
        cpe_str_dup(subPath + pathSize + 1, MAXNAMLEN, dp->d_name);

        if (dp->d_type == DT_DIR) {
            rv = dir_rm_recursion(subPath, em, talloc);
        }
        else {
            rv = file_rm(subPath, em);
        }

        if (rv != 0) break;
    }

    /*remove curent node*/
    if (rv == 0) {
        rv = dir_rm(path, em);
    }

    /*clear resource*/
    if (subPath) mem_free(talloc, subPath);
    dir_close(dirp, em);
    return rv;
}

int dir_is_empty(const char * path, error_monitor_t em) {
    DIR * dirp;
    struct dirent dbuf;
    struct dirent * dp;
    int rv;

    (void)dbuf; /*for vc compile warning*/

    dirp = dir_open(path, 0, NULL);
    if (dirp == NULL) { return 1; }

    rv = readdir_r(dirp, &dbuf, &dp);

    dir_close(dirp, NULL);

    return rv == 0 ? 0 : 1;
}

int dir_exist(const char * path, error_monitor_t em) {
    struct stat buffer;
    int status;
    status = inode_stat_by_path(path, &buffer, ENOENT, em);
    if (status != 0) return 0;

    return S_ISDIR(buffer.st_mode);
}

