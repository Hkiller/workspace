#ifndef CPE_VFS_BUILDER_ENTRY_I_H
#define CPE_VFS_BUILDER_ENTRY_I_H
#include "cpe/utils/buffer.h"
#include "vfs_builder_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vfs_builder_entry {
    vfs_builder_t m_builder;
    vfs_builder_entry_t m_parent;
    TAILQ_ENTRY(vfs_builder_entry) m_next;
    const char * m_name;
    uint8_t m_name_len;
    uint8_t m_is_dir;
    union {
        struct {
            vfs_builder_entry_list_t m_childs;
        } m_dir;
        struct {
            struct mem_buffer m_data;
        } m_file;
    };
};

vfs_builder_entry_t vfs_builder_entry_create(vfs_builder_t builder, vfs_builder_entry_t parent, const char * name, const char * name_end, uint8_t is_dir);
void vfs_builder_entry_free(vfs_builder_entry_t entry);

vfs_builder_entry_t vfs_builder_entry_find_child_by_name(vfs_builder_entry_t parent, const char * name, const char * name_end);
vfs_builder_entry_t vfs_builder_entry_find_child_by_path(vfs_builder_entry_t parent, const char * name, const char * path_end);    
    
#ifdef __cplusplus
}
#endif

#endif
