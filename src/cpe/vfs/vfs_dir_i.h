#ifndef CPE_VFS_DIR_I_H
#define CPE_VFS_DIR_I_H
#include "cpe/vfs/vfs_dir.h"
#include "vfs_manage_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vfs_dir {
    vfs_mgr_t m_mgr;
    vfs_backend_t m_backend;
};

#ifdef __cplusplus
}
#endif

#endif
