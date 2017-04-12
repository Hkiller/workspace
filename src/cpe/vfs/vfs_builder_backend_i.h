#ifndef CPE_VFS_BUILDER_BACKEND_I_H
#define CPE_VFS_BUILDER_BACKEND_I_H
#include "vfs_builder_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vfs_builder_backend {
    vfs_mgr_t m_mgr;
    vfs_builder_list_t m_builders;
};

struct vfs_builder_file {
    vfs_builder_entry_t m_entry;
    ssize_t m_pos;
};

struct vfs_builder_dir {
    vfs_builder_entry_t m_entry;
};

#ifdef __cplusplus
}
#endif

#endif
