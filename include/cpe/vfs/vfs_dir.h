#ifndef CPE_VFS_DIR_H
#define CPE_VFS_DIR_H
#include "vfs_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*dir */
vfs_dir_t vfs_dir_open(vfs_mgr_t mgr, const char * path);
void vfs_dir_close(vfs_dir_t d);

uint8_t vfs_dir_exist(vfs_mgr_t mgr, const char * path);
int vfs_dir_mk(vfs_mgr_t mgr, const char * path);
int vfs_dir_mk_recursion(vfs_mgr_t mgr, const char * path);

void * vfs_dir_data(vfs_dir_t d);

void vfs_dir_entries(vfs_dir_t d, vfs_entry_info_it_t it);
    
#ifdef __cplusplus
}
#endif

#endif
