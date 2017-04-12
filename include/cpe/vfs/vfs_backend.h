#ifndef CPE_VFS_BACKEND_H
#define CPE_VFS_BACKEND_H
#include "vfs_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*vfs_backend_env_clear_fun_t)(void * ctx, void * env);
        
typedef int (*vfs_file_open_fun_t)(void * ctx, void * env, vfs_file_t file, const char * path, const char * mode);
typedef void (*vfs_file_close_fun_t)(void * ctx, vfs_file_t file);
typedef ssize_t (*vfs_file_read_fun_t)(void * ctx, vfs_file_t file, void * buf, size_t size);
typedef ssize_t (*vfs_file_write_fun_t)(void * ctx, vfs_file_t file, const void * buf, size_t size);
typedef int (*vfs_file_seek_fun_t)(void * ctx, vfs_file_t file, ssize_t off, vfs_file_seek_op_t op);
typedef ssize_t (*vfs_file_tell_fun_t)(void * ctx, vfs_file_t file);
typedef uint8_t (*vfs_file_eof_fun_t)(void * ctx, vfs_file_t file);
typedef int (*vfs_file_flush_fun_t)(void * ctx, vfs_file_t file);
typedef ssize_t (*vfs_file_size_fun_t)(void * ctx, vfs_file_t file);
typedef ssize_t (*vfs_file_size_by_path_fun_t)(void * ctx, void * env, const char * path);
typedef uint8_t (*vfs_file_exist_fun_t)(void * ctx, void * env, const char * path);
typedef int (*vfs_file_rm_fun_t)(void * ctx, void * env, const char * path);    

typedef int (*vfs_dir_open_fun_t)(void * ctx, void * env, vfs_dir_t dir, const char * path);
typedef void (*vfs_dir_close_fun_t)(void * ctx, vfs_dir_t dir);
typedef void (*vfs_dir_read_fun_t)(void * ctx, vfs_dir_t dir, vfs_entry_info_it_t it);
typedef uint8_t (*vfs_dir_exist_fun_t)(void * ctx, void * env, const char * path);
typedef int (*vfs_dir_rm_fun_t)(void * ctx, void * env, const char * path);
typedef int (*vfs_dir_mk_fun_t)(void * ctx, void * env, const char * path);    
    
vfs_backend_t vfs_backend_create(
    vfs_mgr_t mgr, const char * name, void * ctx,
    /*env*/
    vfs_backend_env_clear_fun_t env_clear,
    /*file*/
    uint32_t file_capacity, vfs_file_open_fun_t file_open, vfs_file_close_fun_t file_close,
    vfs_file_read_fun_t file_read, vfs_file_write_fun_t file_write, vfs_file_flush_fun_t file_flush,
    vfs_file_seek_fun_t file_seek, vfs_file_tell_fun_t file_tell, vfs_file_eof_fun_t file_eof_p,
    vfs_file_size_fun_t file_size,
    vfs_file_size_by_path_fun_t file_size_by_path,
    vfs_file_exist_fun_t file_exist,
    vfs_file_rm_fun_t file_rm,
    /*dir*/
    uint32_t dir_capacity, vfs_dir_open_fun_t dir_open, vfs_dir_close_fun_t dir_close,
    vfs_dir_read_fun_t dir_read,
    vfs_dir_exist_fun_t dir_exist,
    vfs_dir_rm_fun_t dir_rm,
    vfs_dir_mk_fun_t dir_mk,
    vfs_dir_mk_fun_t dir_mk_recursion);

void vfs_backend_free(vfs_backend_t backend);

vfs_backend_t vfs_backend_find_by_name(vfs_mgr_t mgr, const char * name);
vfs_backend_t vfs_backend_native(vfs_mgr_t mgr);

void * vfs_backend_ctx(vfs_backend_t backend);
    
#ifdef __cplusplus
}
#endif

#endif
