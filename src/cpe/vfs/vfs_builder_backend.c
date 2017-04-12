#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/vfs/vfs_backend.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_dir.h"
#include "cpe/vfs/vfs_entry_info.h"
#include "vfs_builder_backend_i.h"
#include "vfs_builder_entry_i.h"

static int vfs_builder_file_open(void * ctx, void * env, vfs_file_t file, const char * path, const char * mode) {
    vfs_builder_t builder = env;
    vfs_builder_file_t fp = vfs_file_data(file);

    fp->m_entry = vfs_builder_entry_find_child_by_path(builder->m_root, path, path + strlen(path));
    if (fp->m_entry == NULL) {
        if (strchr(mode, 'w')) {
            vfs_builder_entry_t parent;
            const char * name;
            const char * sep = strrchr(path, '/');

            if (sep) {
                parent = vfs_builder_entry_find_child_by_path(builder->m_root, path, sep);
                if (parent == NULL) {
                    CPE_ERROR(builder->m_mgr->m_em, "vfs_builder_file_open: %s owner dir not exist", path);
                    return -1;
                }

                if (!parent->m_is_dir) {
                    CPE_ERROR(builder->m_mgr->m_em, "vfs_builder_file_open: %s owner is not dir", path);
                    return -1;
                }
                
                name = sep + 1;
            }
            else {
                parent = builder->m_root;
                name = path;
            }

            assert(parent);
            fp->m_entry = vfs_builder_entry_create(builder, parent, name, name + strlen(name), 0);
            if (fp->m_entry == NULL) {
                CPE_ERROR(builder->m_mgr->m_em, "vfs_builder_file_open: create entry fail");
                return -1;
            }
        }
        else {
            CPE_ERROR(builder->m_mgr->m_em, "vfs_builder_file_open: %s not exist", path);
            return -1;
        }
    }
    else {
        if (fp->m_entry->m_is_dir) {
            CPE_ERROR(builder->m_mgr->m_em, "vfs_builder_file_open: %s is not file", path);
            return -1;
        }
    }

    if (strchr(mode, 'w')) {
        if (strchr(mode, '+')) {
            fp->m_pos = (ssize_t)mem_buffer_size(&fp->m_entry->m_file.m_data);
        }
        else {
            mem_buffer_clear_data(&fp->m_entry->m_file.m_data);
            fp->m_pos = 0;
        }
    }
    else {
        fp->m_pos = 0;
    }
    
    return 0;
}

static void vfs_builder_file_close(void * ctx, vfs_file_t file) {
}

static ssize_t vfs_builder_file_read(void * ctx, vfs_file_t file, void * buf, size_t size) {
    vfs_builder_backend_t backend = ctx;
    vfs_builder_file_t fp = vfs_file_data(file);
    struct mem_buffer_pos pos;
    size_t buf_size = mem_buffer_size(&fp->m_entry->m_file.m_data);
    size_t read_size;

    if (fp->m_pos == buf_size) return 0;
    if (fp->m_pos > buf_size) {
        CPE_ERROR(
            backend->m_mgr->m_em, "vfs_builder_file_read: pos %d overflow, size=%d",
            (int)fp->m_pos, (int)buf_size);
        return -1;
    }
    
    mem_pos_at(&pos, &fp->m_entry->m_file.m_data, fp->m_pos);

    read_size = buf_size - fp->m_pos;
    if (read_size > size) read_size = size;

    fp->m_pos += read_size;
    return (ssize_t)mem_pos_read(&pos, buf, read_size);
}

static ssize_t vfs_builder_file_write(void * ctx, vfs_file_t file, const void * buf, size_t size) {
    vfs_builder_backend_t backend = ctx;
    vfs_builder_file_t fp = vfs_file_data(file);
    ssize_t writed_size;
    struct mem_buffer_pos pos;
    size_t buf_size = mem_buffer_size(&fp->m_entry->m_file.m_data);

    if (fp->m_pos == buf_size) {
        writed_size = mem_buffer_append(&fp->m_entry->m_file.m_data, buf, size);
    }
    else {
        mem_pos_at(&pos, &fp->m_entry->m_file.m_data, fp->m_pos);
        if (mem_pos_seek(&pos, fp->m_pos) != fp->m_pos) {
            CPE_ERROR(
                backend->m_mgr->m_em, "vfs_builder_file_write: pos %d overflow, size=%d",
                (int)fp->m_pos, (int)mem_buffer_size(&fp->m_entry->m_file.m_data));
            return -1;
        }

        writed_size = mem_pos_write(&pos, buf, size);
    }

    if (writed_size > 0) {
        fp->m_pos += writed_size;
    }

    return writed_size;
}

static int vfs_builder_file_seek(void * ctx, vfs_file_t file, ssize_t off, vfs_file_seek_op_t op) {
    vfs_builder_backend_t backend = ctx;
    vfs_builder_file_t fp = vfs_file_data(file);
    ssize_t new_pos;
    
    switch(op) {
    case vfs_file_seek_set:
        new_pos = off;
        break;
    case vfs_file_seek_cur:
        new_pos = fp->m_pos + off;
        break;
    case vfs_file_seek_end:
        new_pos = ((ssize_t)mem_buffer_size(&fp->m_entry->m_file.m_data)) + off;
        break;
    default:
        CPE_ERROR(backend->m_mgr->m_em, "vfs_builder_file_seek: unknown seek op %d", op);
        return -1;
    }

    if (new_pos < 0 || new_pos > ((ssize_t)mem_buffer_size(&fp->m_entry->m_file.m_data))) {
        CPE_ERROR(backend->m_mgr->m_em, "vfs_builder_file_seek: pos %d overflow, size=%d", (int)new_pos, (int)mem_buffer_size(&fp->m_entry->m_file.m_data));
        return -1;
    }

    fp->m_pos = new_pos;
    return 0;
}

static ssize_t vfs_builder_file_tell(void * ctx, vfs_file_t file) {
    vfs_builder_file_t fp = vfs_file_data(file);
    return fp->m_pos;
}

static uint8_t vfs_builder_file_eof(void * ctx, vfs_file_t file) {
    vfs_builder_file_t fp = vfs_file_data(file);
    return fp->m_pos >= (ssize_t)mem_buffer_size(&fp->m_entry->m_file.m_data) ? 1 : 0;
}

static int vfs_builder_file_flush(void * ctx, vfs_file_t file) {
    return 0;
}

static ssize_t vfs_builder_file_size(void * ctx, vfs_file_t file) {
    vfs_builder_file_t fp = vfs_file_data(file);
    return (ssize_t)mem_buffer_size(&fp->m_entry->m_file.m_data);
}

static ssize_t vfs_builder_file_size_by_path(void * ctx, void * env, const char * path) {
    vfs_builder_backend_t backend = ctx;
    vfs_builder_t builder = env;
    vfs_builder_entry_t entry = vfs_builder_entry_find_child_by_path(builder->m_root, path, path + strlen(path));

    if (entry == NULL) {
        CPE_ERROR(backend->m_mgr->m_em, "vfs_builder_file_size_by_path: file %s not exist!", path);
        return -1;
    }

    return (ssize_t)mem_buffer_size(&entry->m_file.m_data);
}

static uint8_t vfs_builder_file_exist(void * ctx, void * env, const char * path) {
    vfs_builder_t builder = env;
    vfs_builder_entry_t entry;

    entry = vfs_builder_entry_find_child_by_path(builder->m_root, path, path + strlen(path));
    return entry && !entry->m_is_dir ? 1 : 0;
}

static int vfs_builder_file_rm(void * ctx, void * env, const char * path) {
    vfs_builder_backend_t backend = ctx;
    vfs_builder_t builder = env;
    vfs_builder_entry_t entry;

    entry = vfs_builder_entry_find_child_by_path(builder->m_root, path, path + strlen(path));
    if (entry == NULL) {
        CPE_ERROR(backend->m_mgr->m_em, "vfs_builder_file_rm: %s not exist", path);
        return -1;
    }

    if (entry->m_is_dir) {
        CPE_ERROR(backend->m_mgr->m_em, "vfs_builder_file_rm: %s is not file", path);
        return -1;
    }

    vfs_builder_entry_free(entry);
    
    return 0;
}

static int vfs_builder_dir_open(void * ctx, void * env, vfs_dir_t dir, const char * path) {
    vfs_builder_t builder = env;
    vfs_builder_dir_t builder_dir = vfs_dir_data(dir);

    builder_dir->m_entry = vfs_builder_entry_find_child_by_path(builder->m_root, path, path + strlen(path));
    if (builder_dir->m_entry == NULL) {
        CPE_ERROR(builder->m_mgr->m_em, "vfs_builder_dir_open: %s not exist", path);
        return -1;
    }

    if (!builder_dir->m_entry->m_is_dir) {
        CPE_ERROR(builder->m_mgr->m_em, "vfs_builder_dir_open: %s is not dir", path);
        return -1;
    }
        
    return 0;
}

static void vfs_builder_dir_close(void * ctx, vfs_dir_t dir) {
}

struct vfs_builder_dir_it_data {
    vfs_builder_entry_t m_cur;
    struct vfs_entry_info m_entry;
};

static vfs_entry_info_t vfs_builder_dir_it_next(struct vfs_entry_info_it * it) {
    struct vfs_builder_dir_it_data * it_data = (void*)it->m_data;

    if (it_data->m_cur == NULL) return NULL;

    it_data->m_entry.m_name = it_data->m_cur->m_name;
    it_data->m_entry.m_type = it_data->m_cur->m_is_dir ? vfs_entry_dir : vfs_entry_file;

    it_data->m_cur = TAILQ_NEXT(it_data->m_cur, m_next);
    
    return &it_data->m_entry;
}

static void vfs_builder_dir_read(void * ctx, vfs_dir_t dir, vfs_entry_info_it_t it) {
    struct vfs_builder_dir_it_data * it_data = (void*)it->m_data;
    vfs_builder_dir_t builder_dir = vfs_dir_data(dir);

    it->next = vfs_builder_dir_it_next;
    it_data->m_cur = TAILQ_FIRST(&builder_dir->m_entry->m_dir.m_childs);
}

static uint8_t vfs_builder_dir_exist(void * ctx, void * env, const char * path) {
    vfs_builder_t builder = env;
    vfs_builder_entry_t entry;

    entry = vfs_builder_entry_find_child_by_path(builder->m_root, path, path + strlen(path));
    return entry && entry->m_is_dir ? 1 : 0;
}

static int vfs_builder_dir_rm(void * ctx, void * env, const char * path) {
    vfs_builder_backend_t backend = ctx;
    vfs_builder_t builder = env;
    vfs_builder_entry_t entry;

    entry = vfs_builder_entry_find_child_by_path(builder->m_root, path, path + strlen(path));
    if (entry == NULL) {
        CPE_ERROR(backend->m_mgr->m_em, "vfs_builder_dir_rm: %s not exist", path);
        return -1;
    }

    if (!entry->m_is_dir) {
        CPE_ERROR(backend->m_mgr->m_em, "vfs_builder_dir_rm: %s is not dir", path);
        return -1;
    }

    vfs_builder_entry_free(entry);

    return 0;
}

static int vfs_builder_dir_mk(void * ctx, void * env, const char * path) {
    vfs_builder_t builder = env;
    vfs_builder_entry_t parent;
    const char * sep;
    const char * name;

    sep = strrchr(path, '/');
    if (sep) {
        parent = vfs_builder_entry_find_child_by_path(builder->m_root, path, sep);
        if (parent == NULL) {
            CPE_ERROR(builder->m_mgr->m_em, "vfs_builder_dir_mk: %s owner dir not exist", path);
            return -1;
        }

        if (!parent->m_is_dir) {
            CPE_ERROR(builder->m_mgr->m_em, "vfs_builder_dir_mk: %s owner is not dir", path);
            return -1;
        }
                
        name = sep + 1;
    }
    else {
        parent = builder->m_root;
        name = path;
    }

    assert(parent);
    if (vfs_builder_entry_create(builder, parent, name, name + strlen(name), 1) == NULL) {
        CPE_ERROR(builder->m_mgr->m_em, "vfs_builder_dir_mk: create entry fail");
        return -1;
    }

    return 0;
}

static int vfs_builder_dir_mk_recursion(void * ctx, void * env, const char * path) {
    vfs_builder_t builder = env;
    vfs_builder_entry_t parent;
    vfs_builder_entry_t cur;
    const char * sep;

    parent = builder->m_root;
    while(path[0]) {
        sep = strchr(path, '/');
        if (sep == NULL) sep = path + strlen(path);
        
        cur = vfs_builder_entry_find_child_by_name(parent, path, sep);
        if (cur == NULL) {
            cur = vfs_builder_entry_create(builder, parent, path, sep, 1);
            if (cur == NULL) {
                CPE_ERROR(builder->m_mgr->m_em, "vfs_builder_dir_mk_recursion: create entry fail");
                return -1;
            }
        }
        else if (!cur->m_is_dir) {
            CPE_ERROR(builder->m_mgr->m_em, "vfs_builder_dir_mk_recursion: %s is not dir", path);
            return -1;
        }
        
        parent = cur;
        if (*sep == 0) break;
        path = sep + 1;
    }

    return 0;
}

vfs_backend_t vfs_builder_create_backend(vfs_mgr_t mgr) {
    vfs_builder_backend_t builder_backend;
    vfs_backend_t backend;
    
    builder_backend = mem_alloc(mgr->m_alloc, sizeof(struct vfs_builder_backend));
    if (builder_backend == NULL) {
        CPE_ERROR(mgr->m_em, "vfs_builder_create_backend: alloc builder backend fail!");
        return NULL;
    }

    builder_backend->m_mgr = mgr;
    TAILQ_INIT(&builder_backend->m_builders);
    
    backend =
        vfs_backend_create(
            mgr, "builder:", builder_backend,
            /*env*/
            NULL,
            /*file*/
            sizeof(struct vfs_builder_file), vfs_builder_file_open, vfs_builder_file_close,
            vfs_builder_file_read, vfs_builder_file_write, vfs_builder_file_flush,
            vfs_builder_file_seek, vfs_builder_file_tell, vfs_builder_file_eof,
            vfs_builder_file_size,
            vfs_builder_file_size_by_path,
            vfs_builder_file_exist,
            vfs_builder_file_rm,
            /*dir*/
            sizeof(struct vfs_builder_dir), vfs_builder_dir_open, vfs_builder_dir_close, vfs_builder_dir_read,
            vfs_builder_dir_exist,
            vfs_builder_dir_rm,
            vfs_builder_dir_mk,
            vfs_builder_dir_mk_recursion);
    if (backend == NULL) {
        mem_free(mgr->m_alloc, builder_backend);
        return NULL;
    }

    return backend;
}
