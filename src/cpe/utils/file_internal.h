#ifndef CPE_FILE_PRIVATE_H
#define CPE_FILE_PRIVATE_H
#include <sys/stat.h>
#include <errno.h>
#include "cpe/pal/pal_dirent.h"
#include "cpe/utils/file.h"

#ifdef __cplusplus
extern "C" {
#endif

DIR * dir_open(const char * path, int ignoreError, error_monitor_t em);
void dir_close(DIR * dirp, error_monitor_t em);
int inode_stat_by_path(const char * path, struct stat * buf, int ignoreError, error_monitor_t em);
int inode_stat_by_fileno(int fno, struct stat * buf, int ignoreError, error_monitor_t em);


#ifdef __cplusplus
}
#endif

#endif

