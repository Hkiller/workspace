#ifndef CPE_VFS_MANAGE_H
#define CPE_VFS_MANAGE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "vfs_types.h"

#ifdef __cplusplus
extern "C" {
#endif

vfs_mgr_t vfs_mgr_create(mem_allocrator_t alloc, error_monitor_t em);
void vfs_mgr_free(vfs_mgr_t em);

vfs_mount_point_t vfs_mgr_root_point(vfs_mgr_t mgr);
vfs_mount_point_t vfs_mgr_current_point(vfs_mgr_t mgr);

#ifdef __cplusplus
}
#endif

#endif
