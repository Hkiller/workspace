#ifndef CPE_VFS_FILE_H
#define CPE_VFS_FILE_H
#include "cpe/utils/utils_types.h"
#include "vfs_types.h"

#ifdef __cplusplus
extern "C" {
#endif

vfs_file_t vfs_file_open(vfs_mgr_t mgr, const char * path, const char * mod);
void vfs_file_close(vfs_file_t f);

uint8_t vfs_file_exist(vfs_mgr_t mgr, const char * path);
int vfs_file_rm(vfs_mgr_t mgr, const char * path);
    
void * vfs_file_data(vfs_file_t f);
ssize_t vfs_file_read(vfs_file_t f, void * buf, size_t size);
ssize_t vfs_file_write(vfs_file_t f, void const * buf, size_t size);
int vfs_file_flush(vfs_file_t f);
int vfs_file_seek(vfs_file_t f, ssize_t off, vfs_file_seek_op_t op);
ssize_t vfs_file_tell(vfs_file_t f);
uint8_t vfs_file_eof(vfs_file_t f);

ssize_t vfs_file_load_to_buffer(mem_buffer_t buffer, vfs_file_t f);
ssize_t vfs_file_load_to_buffer_by_path(mem_buffer_t buffer, vfs_mgr_t mgr, const char * path);    
ssize_t vfs_file_write_from_buffer(vfs_file_t f, mem_buffer_t buffer);

int vfs_file_calc_md5(vfs_file_t file, cpe_md5_value_t md5);
int vfs_file_calc_md5_by_path(vfs_mgr_t mgr, const char * path, cpe_md5_value_t md5);    

ssize_t vfs_file_size(vfs_file_t f);
ssize_t vfs_file_size_by_path(vfs_mgr_t mgr, const char * path);

int vfs_file_copy(vfs_mgr_t vfs, const char * output, const char * input);

uint8_t vfs_file_check_md5_str(vfs_mgr_t mgr, const char * path, const char * md5);
uint8_t vfs_file_check_md5(vfs_mgr_t mgr, const char * path, cpe_md5_value_t md5);

#ifdef __cplusplus
}
#endif

#endif
