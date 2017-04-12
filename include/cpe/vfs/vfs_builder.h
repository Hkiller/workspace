#ifndef CPE_VFS_BUILDER_H
#define CPE_VFS_BUILDER_H
#include "vfs_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*builder */
vfs_builder_t vfs_builder_create(vfs_mgr_t mgr, const char * path);
void vfs_builder_free(vfs_builder_t builder);

const char * vfs_builder_path(vfs_builder_t builder);
    
/*entry*/
vfs_builder_entry_t vfs_builder_entry_find(vfs_builder_t builder, const char * path);

#ifdef __cplusplus
}
#endif

#endif
