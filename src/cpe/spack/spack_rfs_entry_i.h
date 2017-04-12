#ifndef CPE_SPACK_RFS_ENTRY_I_H
#define CPE_SPACK_RFS_ENTRY_I_H
#include "spack_rfs_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct spack_rfs_entry {
    spack_rfs_t m_rfs;
    spack_rfs_entry_t m_parent;
    TAILQ_ENTRY(spack_rfs_entry) m_next;
    const char * m_name;
    size_t m_name_len;
    uint8_t m_is_dir;
    union {
        struct {
            spack_rfs_entry_list_t m_childs;
        } m_dir;
        struct {
            uint32_t m_start;
            uint32_t m_size;
        } m_file;
    };
};

spack_rfs_entry_t spack_rfs_entry_create(spack_rfs_t rfs, spack_rfs_entry_t parent, const char * name, const char * name_end, uint8_t is_dir);
void spack_rfs_entry_free(spack_rfs_entry_t entry);

spack_rfs_entry_t spack_rfs_entry_find_child_by_name(spack_rfs_entry_t parent, const char * name, const char * name_end);
spack_rfs_entry_t spack_rfs_entry_find_child_by_path(spack_rfs_entry_t parent, const char * name, const char * path_end);    

int spack_rfs_entry_file_create(spack_rfs_t rfs, const char * name, uint32_t start, uint32_t size);
    
#ifdef __cplusplus
}
#endif

#endif
