#ifndef CPE_VFS_TYPES_H
#define CPE_VFS_TYPES_H
#include "cpe/pal/pal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vfs_mgr * vfs_mgr_t;
typedef struct vfs_file * vfs_file_t;
typedef struct vfs_dir * vfs_dir_t;
typedef struct vfs_entry_info * vfs_entry_info_t;
typedef struct vfs_entry_info_it * vfs_entry_info_it_t;

typedef struct vfs_mount_point * vfs_mount_point_t;
    
typedef struct vfs_backend * vfs_backend_t;
typedef struct vfs_read_stream * vfs_read_stream_t;
typedef struct vfs_write_stream * vfs_write_stream_t;
typedef struct vfs_visitor * vfs_visitor_t;

typedef struct vfs_builder * vfs_builder_t;
typedef struct vfs_builder_entry * vfs_builder_entry_t;
typedef struct vfs_builder_entry_it * vfs_builder_entry_it_t;

typedef enum vfs_visitor_next_op {
    vfs_visitor_next_go
    , vfs_visitor_next_ignore
    , vfs_visitor_next_exit
} vfs_visitor_next_op_t;

typedef enum vfs_entry_type {
    vfs_entry_file
    , vfs_entry_dir
} vfs_entry_type_t;

typedef enum vfs_file_seek_op {
    vfs_file_seek_set = 1,
    vfs_file_seek_cur,
    vfs_file_seek_end,
} vfs_file_seek_op_t;
    
#ifdef __cplusplus
}
#endif

#endif
