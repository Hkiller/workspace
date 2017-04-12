#ifndef CPE_VFS_BACKEND_I_H
#define CPE_VFS_BACKEND_I_H
#include "cpe/vfs/vfs_backend.h"
#include "vfs_manage_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vfs_backend {
    vfs_mgr_t m_mgr;
    TAILQ_ENTRY(vfs_backend) m_next;
    char m_name[32];
    vfs_mount_point_list_t m_mount_points;
    void * m_ctx;
    vfs_backend_env_clear_fun_t m_env_clear;
    uint32_t m_file_capacity;
    vfs_file_open_fun_t m_file_open;
    vfs_file_close_fun_t m_file_close;
    vfs_file_read_fun_t m_file_read;
    vfs_file_write_fun_t m_file_write;
    vfs_file_flush_fun_t m_file_flush;
    vfs_file_seek_fun_t m_file_seek;
    vfs_file_tell_fun_t m_file_tell;
    vfs_file_eof_fun_t m_file_eof;
    vfs_file_size_fun_t m_file_size;
    vfs_file_size_by_path_fun_t m_file_size_by_path;
    vfs_file_exist_fun_t m_file_exist;
    vfs_file_rm_fun_t m_file_rm;
    uint32_t m_dir_capacity;
    vfs_dir_open_fun_t m_dir_open;
    vfs_dir_close_fun_t m_dir_close;
    vfs_dir_read_fun_t m_dir_read;
    vfs_dir_exist_fun_t m_dir_exist;
    vfs_dir_rm_fun_t m_dir_rm;
    vfs_dir_mk_fun_t m_dir_mk;
    vfs_dir_mk_fun_t m_dir_mk_recursion;
};

int vfs_backend_native_create(vfs_mgr_t mgr);
    
#ifdef __cplusplus
}
#endif

#endif
