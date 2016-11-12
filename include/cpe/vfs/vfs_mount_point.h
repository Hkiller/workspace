#ifndef CPE_VFS_MOUNT_POINT_H
#define CPE_VFS_MOUNT_POINT_H
#include "vfs_types.h"

#ifdef __cplusplus
extern "C" {
#endif

vfs_mount_point_t vfs_mount_point_mount(vfs_mount_point_t from, const char * path, void * backend_env, vfs_backend_t backend);
int vfs_mount_point_unmount(vfs_mount_point_t from, const char * path);

vfs_mount_point_t vfs_mount_point_find_by_path(vfs_mgr_t mgr, const char * * path);
vfs_mount_point_t vfs_mount_point_find_child_by_path(vfs_mount_point_t mount_point, const char * * path);
vfs_mount_point_t vfs_mount_point_find_child_by_name(vfs_mount_point_t p, const char * name);
    
#ifdef __cplusplus
}
#endif

#endif
