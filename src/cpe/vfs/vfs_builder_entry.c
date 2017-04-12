#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "vfs_builder_entry_i.h"

vfs_builder_entry_t
vfs_builder_entry_create(
    vfs_builder_t builder, vfs_builder_entry_t parent, const char * name, const char * name_end, uint8_t is_dir)
{
    vfs_builder_entry_t entry;
    size_t name_len = name_end - name;
    
    entry = mem_alloc(builder->m_mgr->m_alloc, sizeof(struct vfs_builder_entry) + name_len + 1);
    if (entry == NULL) {
        CPE_ERROR(builder->m_mgr->m_em, "vfs_builder_entry_create: alloc fail!");
        return NULL;
    }

    memcpy(entry + 1, name, name_len);
    ((char *)(entry + 1))[name_len] = 0;

    entry->m_builder = builder;
    entry->m_parent = parent;
    entry->m_name = (void*)(entry + 1);
    entry->m_name_len = name_len;
    entry->m_is_dir = is_dir;

    if (is_dir) {
        TAILQ_INIT(&entry->m_dir.m_childs);
    }
    else {
        mem_buffer_init(&entry->m_file.m_data, builder->m_mgr->m_alloc);
    }
    
    if (parent) {
        assert(parent->m_is_dir);
        TAILQ_INSERT_TAIL(&parent->m_dir.m_childs, entry, m_next);
    }
    
    return entry;
}

void vfs_builder_entry_free(vfs_builder_entry_t entry) {
    if (entry->m_is_dir) {
        while(!TAILQ_EMPTY(&entry->m_dir.m_childs)) {
            vfs_builder_entry_free(TAILQ_FIRST(&entry->m_dir.m_childs));
        }
    }
    else {
        mem_buffer_clear(&entry->m_file.m_data);
    }

    if (entry->m_parent) {
        assert(entry->m_parent->m_is_dir);
        TAILQ_REMOVE(&entry->m_parent->m_dir.m_childs, entry, m_next);
    }

    mem_free(entry->m_builder->m_mgr->m_alloc, entry);
}

vfs_builder_entry_t
vfs_builder_entry_find_child_by_name(vfs_builder_entry_t parent, const char * name, const char * name_end) {
    vfs_builder_entry_t entry;

    assert(parent->m_is_dir);
    if (!parent->m_is_dir) return NULL;

    TAILQ_FOREACH(entry, &parent->m_dir.m_childs, m_next) {
        if (name + entry->m_name_len != name_end) continue;
        if (memcmp(name, entry->m_name, entry->m_name_len) == 0) return entry;
    }

    return NULL;
}

vfs_builder_entry_t
vfs_builder_entry_find_child_by_path(vfs_builder_entry_t parent, const char * path, const char * path_end) {
    const char * sep;

    for(sep = memchr(path, '/', path_end - path);
        sep && sep < path_end;
        path = sep + 1, sep = memchr(path, '/', path_end - path))
    {
        if (sep > path) {
            parent = vfs_builder_entry_find_child_by_name(parent, path, sep);
            if (parent == NULL) return NULL;
        }
    }

    if (path < path_end) {
        parent = vfs_builder_entry_find_child_by_name(parent, path, path_end);
    }

    return parent;
}

