#ifndef CPE_VFS_BUILDER_I_H
#define CPE_VFS_BUILDER_I_H
#include "cpe/vfs/vfs_builder.h"
#include "vfs_manage_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vfs_builder_backend * vfs_builder_backend_t;
typedef struct vfs_builder_file * vfs_builder_file_t;
typedef struct vfs_builder_dir * vfs_builder_dir_t;

typedef TAILQ_HEAD(vfs_builder_list, vfs_builder) vfs_builder_list_t;
typedef TAILQ_HEAD(vfs_builder_entry_list, vfs_builder_entry) vfs_builder_entry_list_t;
    
struct vfs_builder {
    vfs_mgr_t m_mgr;
    vfs_builder_backend_t m_backend;
    TAILQ_ENTRY(vfs_builder) m_next;
    vfs_mount_point_t m_mount_point;
    vfs_builder_entry_t m_root;
};

vfs_backend_t vfs_builder_create_backend(vfs_mgr_t mgr);

#ifdef __cplusplus
}
#endif

#endif
