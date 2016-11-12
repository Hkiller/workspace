#ifndef CPE_VFS_FILE_I_H
#define CPE_VFS_FILE_I_H
#include "cpe/vfs/vfs_file.h"
#include "vfs_manage_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vfs_file {
    vfs_mgr_t m_mgr;
    vfs_backend_t m_backend;
};

#ifdef __cplusplus
}
#endif

#endif
