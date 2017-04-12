#ifndef CPE_VFS_MANAGE_I_H
#define CPE_VFS_MANAGE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "cpe/vfs/vfs_manage.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(vfs_file_list, vfs_file) vfs_file_list_t;
typedef TAILQ_HEAD(vfs_dir_list, vfs_dir) vfs_dir_list_t;    
typedef TAILQ_HEAD(vfs_backend_list, vfs_backend) vfs_backend_list_t;
typedef TAILQ_HEAD(vfs_mount_point_list, vfs_mount_point) vfs_mount_point_list_t;
    
struct vfs_mgr {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;

    vfs_backend_list_t m_backends;

    vfs_mount_point_t m_root;
    vfs_mount_point_t m_current;
    
    vfs_mount_point_list_t m_free_mount_points;

    struct mem_buffer m_search_path_buffer;

    struct mem_buffer m_tmp_buffer;
};
    
#ifdef __cplusplus
}
#endif

#endif
