#ifndef CPE_VFS_ENTRY_INFO_H
#define CPE_VFS_ENTRY_INFO_H
#include "vfs_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vfs_entry_info {
    const char * m_name;
    vfs_entry_type_t m_type;
};
    
struct vfs_entry_info_it {
    vfs_entry_info_t (*next)(struct vfs_entry_info_it * it);
    char m_data[64];
};

#define vfs_entry_info_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
