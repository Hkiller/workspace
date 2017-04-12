#ifndef CPE_VFS_VISITOR_H
#define CPE_VFS_VISITOR_H
#include "vfs_types.h"

#ifdef __cplusplus
extern "C" {
#endif
    
struct vfs_visitor {
    /*ignore: ignore this dir, but leave will be still called*/
    /*exit: exit direct*/
    vfs_visitor_next_op_t (*on_dir_enter)(vfs_mgr_t mgr, const char * full, const char * base, void * ctx);
    /*exit: exit direct*/
    vfs_visitor_next_op_t (*on_dir_leave)(vfs_mgr_t mgr, const char * full, const char * base, void * ctx);
    /*exit: exit direct*/
    vfs_visitor_next_op_t (*on_file)(vfs_mgr_t mgr, const char * full, const char * f, void * ctx);
};

void vfs_search_dir(vfs_mgr_t mgr, vfs_visitor_t visitor, void * ctx, const char * path, int max_level);

#ifdef __cplusplus
}
#endif

#endif
