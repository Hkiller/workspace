#ifndef CPE_CFG_VFS_H
#define CPE_CFG_VFS_H
#include "cpe/utils/buffer.h"
#include "cpe/vfs/vfs_types.h"
#include "cfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int cfg_vfs_read_bin_file(cfg_t cfg, vfs_mgr_t vfs, const char * path, error_monitor_t em);
int cfg_vfs_read_file(cfg_t cfg, vfs_mgr_t vfs, const char * path, error_monitor_t em);
int cfg_vfs_read_dir(cfg_t cfg, vfs_mgr_t vfs, const char * path, error_monitor_t em);    

int cfg_vfs_write_bin_file(void);

#ifdef __cplusplus
}
#endif

#endif


