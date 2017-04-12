#ifndef CPE_VFS_MOUNT_POINT_I_H
#define CPE_VFS_MOUNT_POINT_I_H
#include "cpe/vfs/vfs_mount_point.h"
#include "vfs_manage_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vfs_mount_point {
    vfs_mgr_t m_mgr;
    char m_name[32];
    size_t m_name_len;

    /*tree*/
    vfs_mount_point_t m_parent;
    TAILQ_ENTRY(vfs_mount_point) m_next_for_parent;
    vfs_mount_point_list_t m_childs;
    
    /*bridger*/
    vfs_mount_point_t m_bridger_to;
    TAILQ_ENTRY(vfs_mount_point) m_next_bridger_to;
    vfs_mount_point_list_t m_bridger_froms;

    /*backend*/
    void * m_backend_env;
    vfs_backend_t m_backend;
    TAILQ_ENTRY(vfs_mount_point) m_next_for_backend;
};

vfs_mount_point_t vfs_mount_point_create(vfs_mgr_t mgr, const char * name, const char * name_end, vfs_mount_point_t parent, void * backend_env, vfs_backend_t backend);
void vfs_mount_point_free(vfs_mount_point_t mount_point);
void vfs_mount_point_real_free(vfs_mount_point_t mount_point);
    
vfs_mount_point_t vfs_mount_point_child_find_by_name_ex(vfs_mount_point_t parent, const char * name, const char * name_end);
vfs_mount_point_t vfs_mount_point_child_find_by_path_ex(vfs_mount_point_t parent, const char * path, const char * path_end);

void vfs_mount_point_set_backend(vfs_mount_point_t mount_point, void * backend_env, vfs_backend_t backend);
void vfs_mount_point_clear_path(vfs_mount_point_t mount_point);

#ifdef __cplusplus
}
#endif

#endif
