#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "spack_rfs_entry_i.h"

spack_rfs_entry_t
spack_rfs_entry_create(
    spack_rfs_t rfs, spack_rfs_entry_t parent, const char * name, const char * name_end, uint8_t is_dir)
{
    spack_rfs_entry_t entry;
    
    entry = mem_alloc(rfs->m_alloc, sizeof(struct spack_rfs_entry));
    if (entry == NULL) {
        CPE_ERROR(rfs->m_em, "spack_rfs_entry_create: alloc fail!");
        return NULL;
    }

    entry->m_rfs = rfs;
    entry->m_parent = parent;
    entry->m_name = name;
    entry->m_name_len = name_end - name;
    entry->m_is_dir = is_dir;

    if (is_dir) {
        TAILQ_INIT(&entry->m_dir.m_childs);
    }
    else {
        entry->m_file.m_start = 0;
        entry->m_file.m_size = 0;
    }
    
    if (parent) {
        assert(parent->m_is_dir);
        TAILQ_INSERT_TAIL(&parent->m_dir.m_childs, entry, m_next);
    }
    
    return entry;
}

void spack_rfs_entry_free(spack_rfs_entry_t entry) {
    if (entry->m_is_dir) {
        while(!TAILQ_EMPTY(&entry->m_dir.m_childs)) {
            spack_rfs_entry_free(TAILQ_FIRST(&entry->m_dir.m_childs));
        }
    }
    else {
    }

    if (entry->m_parent) {
        assert(entry->m_parent->m_is_dir);
        TAILQ_REMOVE(&entry->m_parent->m_dir.m_childs, entry, m_next);
    }

    mem_free(entry->m_rfs->m_alloc, entry);
}

spack_rfs_entry_t
spack_rfs_entry_find_child_by_name(spack_rfs_entry_t parent, const char * name, const char * name_end) {
    spack_rfs_entry_t entry;

    assert(parent->m_is_dir);
    if (!parent->m_is_dir) return NULL;

    TAILQ_FOREACH(entry, &parent->m_dir.m_childs, m_next) {
        if (name + entry->m_name_len != name_end) continue;
        if (memcmp(name, entry->m_name, entry->m_name_len) == 0) return entry;
    }

    return NULL;
}

spack_rfs_entry_t
spack_rfs_entry_find_child_by_path(spack_rfs_entry_t parent, const char * path, const char * path_end) {
    const char * sep;

    for(sep = strchr(path, '/');
        sep && sep < path_end;
        path = sep + 1, sep = strchr(path, '/'))
    {
        if (sep > path) {
            parent = spack_rfs_entry_find_child_by_name(parent, path, sep);
            if (parent == NULL) return NULL;
        }
    }

    if (path < path_end) {
        parent = spack_rfs_entry_find_child_by_name(parent, path, path_end);
    }

    return parent;
}

int spack_rfs_entry_file_create(spack_rfs_t rfs, const char * path, uint32_t start, uint32_t size) {
    spack_rfs_entry_t parent;
    spack_rfs_entry_t cur;
    const char * sep;

    parent = rfs->m_root;
    while((sep = strchr(path, '/'))) {
        if (sep > path) {
            cur = spack_rfs_entry_find_child_by_name(parent, path, sep);
            if (cur == NULL) {
                cur = spack_rfs_entry_create(rfs, parent, path, sep, 1);
                if (cur == NULL) {
                    CPE_ERROR(rfs->m_em, "spack_rfs_entry_file_cr: create entry fail");
                    return -1;
                }
            }
            else if (!cur->m_is_dir) {
                CPE_ERROR(rfs->m_em, "spack_rfs_entry_file_create: %s is not dir", path);
                return -1;
            }
        
            parent = cur;
        }
        
        path = sep + 1;
    }

    if (path[0] == 0) {
        CPE_ERROR(rfs->m_em, "spack_rfs_entry_file_create: no file name");
        return -1;
    }
    
    cur = spack_rfs_entry_create(rfs, parent, path, path + strlen(path), 0);
    if (cur == NULL) {
        CPE_ERROR(rfs->m_em, "spack_rfs_entry_file_create: create file entry fail");
        return -1;
    }

    cur->m_file.m_start = start;
    cur->m_file.m_size = size;
    
    return 0;
}
