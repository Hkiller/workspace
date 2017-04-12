#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/vfs/vfs_backend.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_dir.h"
#include "cpe/vfs/vfs_entry_info.h"
#include "spack_rfs_backend_i.h"
#include "spack_rfs_entry_i.h"

static int spack_rfs_file_open(void * ctx, void * env, vfs_file_t file, const char * path, const char * mode) {
    spack_rfs_t rfs = env;
    spack_rfs_file_t fp = vfs_file_data(file);

    fp->m_entry = spack_rfs_entry_find_child_by_path(rfs->m_root, path, path + strlen(path));
    if (fp->m_entry == NULL) {
        if (strchr(mode, 'w')) {
            spack_rfs_entry_t parent;
            const char * name;
            const char * sep = strrchr(path, '/');
            
            if (sep) {
                parent = spack_rfs_entry_find_child_by_path(rfs->m_root, path, sep);
                if (parent == NULL) {
                    CPE_ERROR(rfs->m_em, "spack_rfs_file_open: %s owner dir not exist", path);
                    return -1;
                }

                if (!parent->m_is_dir) {
                    CPE_ERROR(rfs->m_em, "spack_rfs_file_open: %s owner is not dir", path);
                    return -1;
                }
                
                name = sep + 1;
            }
            else {
                parent = rfs->m_root;
                name = path;
            }

            assert(parent);
            fp->m_entry = spack_rfs_entry_create(rfs, parent, name, name + strlen(name), 0);
            if (fp->m_entry == NULL) {
                CPE_ERROR(rfs->m_em, "spack_rfs_file_open: create entry fail");
                return -1;
            }
        }
        else {
            CPE_ERROR(rfs->m_em, "spack_rfs_file_open: %s not exist", path);
            return -1;
        }
    }
    else {
        if (fp->m_entry->m_is_dir) {
            CPE_ERROR(rfs->m_em, "spack_rfs_file_open: %s is not file", path);
            return -1;
        }
    }

    if (strchr(mode, 'w')) {
        CPE_ERROR(rfs->m_em, "spack_rfs_file_open: not support write file");
        return -1;
    }
    else {
        fp->m_pos = 0;
    }
    
    return 0;
}

static void spack_rfs_file_close(void * ctx, vfs_file_t file) {
}

static ssize_t spack_rfs_file_read(void * ctx, vfs_file_t file, void * buf, size_t size) {
    spack_rfs_backend_t backend = ctx;
    spack_rfs_file_t fp = vfs_file_data(file);
    size_t buf_size = fp->m_entry->m_file.m_size;
    size_t read_size;

    if (fp->m_pos == buf_size) return 0;
    if (fp->m_pos > buf_size) {
        CPE_ERROR(
            backend->m_em, "spack_rfs_file_read: pos %d overflow, size=%d",
            (int)fp->m_pos, (int)buf_size);
        return -1;
    }

    read_size = buf_size - fp->m_pos;
    if (read_size > size) read_size = size;

    memcpy(buf, ((const char *)fp->m_entry->m_rfs->m_data) + fp->m_entry->m_file.m_start + fp->m_pos, read_size);

    fp->m_pos += read_size;

    return (ssize_t)read_size;
}

static ssize_t spack_rfs_file_write(void * ctx, vfs_file_t file, const void * buf, size_t size) {
    spack_rfs_backend_t backend = ctx;
    CPE_ERROR(backend->m_em, "spack_rfs_file_write: spack_rfs not support write!");
    return -1;
}

static int spack_rfs_file_seek(void * ctx, vfs_file_t file, ssize_t off, vfs_file_seek_op_t op) {
    spack_rfs_backend_t backend = ctx;
    spack_rfs_file_t fp = vfs_file_data(file);
    ssize_t new_pos;
    
    switch(op) {
    case vfs_file_seek_set:
        new_pos = off;
        break;
    case vfs_file_seek_cur:
        new_pos = fp->m_pos + off;
        break;
    case vfs_file_seek_end:
        new_pos = ((ssize_t)fp->m_entry->m_file.m_size) + off;
        break;
    default:
        CPE_ERROR(backend->m_em, "spack_rfs_file_seek: unknown seek op %d", op);
        return -1;
    }

    if (new_pos < 0 || new_pos > ((ssize_t)fp->m_entry->m_file.m_size)) {
        CPE_ERROR(backend->m_em, "spack_rfs_file_seek: pos %d overflow, size=%d", (int)new_pos, (int)fp->m_entry->m_file.m_size);
        return -1;
    }

    fp->m_pos = new_pos;
    return 0;
}

static ssize_t spack_rfs_file_tell(void * ctx, vfs_file_t file) {
    spack_rfs_file_t fp = vfs_file_data(file);
    return fp->m_pos;
}

static uint8_t spack_rfs_file_eof(void * ctx, vfs_file_t file) {
    spack_rfs_file_t fp = vfs_file_data(file);
    return fp->m_pos >= (ssize_t)fp->m_entry->m_file.m_size ? 1 : 0;
}

static int spack_rfs_file_flush(void * ctx, vfs_file_t file) {
    return 0;
}

static ssize_t spack_rfs_file_size(void * ctx, vfs_file_t file) {
    spack_rfs_file_t fp = vfs_file_data(file);
    return (ssize_t)fp->m_entry->m_file.m_size;
}

static ssize_t spack_rfs_file_size_by_path(void * ctx, void * env, const char * path) {
    spack_rfs_backend_t backend = ctx;
    spack_rfs_t rfs = env;
    spack_rfs_entry_t entry = spack_rfs_entry_find_child_by_path(rfs->m_root, path, path + strlen(path));

    if (entry == NULL) {
        CPE_ERROR(backend->m_em, "spack_rfs_file_size_by_path: file %s not exist!", path);
        return -1;
    }

    return (ssize_t)entry->m_file.m_size;
}

static uint8_t spack_rfs_file_exist(void * ctx, void * env, const char * path) {
    spack_rfs_t rfs = env;
    spack_rfs_entry_t entry;

    entry = spack_rfs_entry_find_child_by_path(rfs->m_root, path, path + strlen(path));
    return entry && !entry->m_is_dir ? 1 : 0;
}

static int spack_rfs_file_rm(void * ctx, void * env, const char * path) {
    spack_rfs_backend_t backend = ctx;
    CPE_ERROR(backend->m_em, "spack_rfs_file_rm: rfs not support rm file");
    return -1;
}

static int spack_rfs_dir_open(void * ctx, void * env, vfs_dir_t dir, const char * path) {
    spack_rfs_t rfs = env;
    spack_rfs_dir_t rfs_dir = vfs_dir_data(dir);

    rfs_dir->m_entry = spack_rfs_entry_find_child_by_path(rfs->m_root, path, path + strlen(path));
    if (rfs_dir->m_entry == NULL) {
        CPE_ERROR(rfs->m_em, "spack_rfs_dir_open: %s not exist", path);
        return -1;
    }

    if (!rfs_dir->m_entry->m_is_dir) {
        CPE_ERROR(rfs->m_em, "spack_rfs_dir_open: %s is not dir", path);
        return -1;
    }
        
    return 0;
}

static void spack_rfs_dir_close(void * ctx, vfs_dir_t dir) {
}

struct spack_rfs_dir_it_data {
    spack_rfs_entry_t m_cur;
    struct vfs_entry_info m_entry;
};

static vfs_entry_info_t spack_rfs_dir_it_next(struct vfs_entry_info_it * it) {
    struct spack_rfs_dir_it_data * it_data = (void*)it->m_data;

    if (it_data->m_cur == NULL) return NULL;

    it_data->m_entry.m_name = it_data->m_cur->m_name;
    it_data->m_entry.m_type = it_data->m_cur->m_is_dir ? vfs_entry_dir : vfs_entry_file;

    it_data->m_cur = TAILQ_NEXT(it_data->m_cur, m_next);
    
    return &it_data->m_entry;
}

static void spack_rfs_dir_read(void * ctx, vfs_dir_t dir, vfs_entry_info_it_t it) {
    struct spack_rfs_dir_it_data * it_data = (void*)it->m_data;
    spack_rfs_dir_t rfs_dir = vfs_dir_data(dir);

    it->next = spack_rfs_dir_it_next;
    it_data->m_cur = TAILQ_FIRST(&rfs_dir->m_entry->m_dir.m_childs);
}

static uint8_t spack_rfs_dir_exist(void * ctx, void * env, const char * path) {
    spack_rfs_t rfs = env;
    spack_rfs_entry_t entry;

    entry = spack_rfs_entry_find_child_by_path(rfs->m_root, path, path + strlen(path));
    return entry && entry->m_is_dir ? 1 : 0;
}

static int spack_rfs_dir_rm(void * ctx, void * env, const char * path) {
    spack_rfs_backend_t backend = ctx;
    CPE_ERROR(backend->m_em, "spack_rfs_dir_rm: rfs not support dir rm");
    return -1;
}

static int spack_rfs_dir_mk(void * ctx, void * env, const char * path) {
    spack_rfs_t rfs = env;
    CPE_ERROR(rfs->m_em, "spack_rfs_dir_mk: rfs not support create dir");
    return -1;
}

static int spack_rfs_dir_mk_recursion(void * ctx, void * env, const char * path) {
    spack_rfs_t rfs = env;
    CPE_ERROR(rfs->m_em, "spack_rfs_dir_mk_recursion: rfs not support create dir");
    return -1;
}

vfs_backend_t spack_rfs_create_backend(vfs_mgr_t vfs, mem_allocrator_t alloc, error_monitor_t em) {
    spack_rfs_backend_t rfs_backend;
    vfs_backend_t backend;
    
    rfs_backend = mem_alloc(alloc, sizeof(struct spack_rfs_backend));
    if (rfs_backend == NULL) {
        CPE_ERROR(em, "spack_rfs_create_backend: alloc rfs backend fail!");
        return NULL;
    }

    rfs_backend->m_alloc = alloc;
    rfs_backend->m_em = em;
    TAILQ_INIT(&rfs_backend->m_rfss);
    
    backend =
        vfs_backend_create(
            vfs, "spack_rfs:", rfs_backend,
            /*env*/
            NULL,
            /*file*/
            sizeof(struct spack_rfs_file), spack_rfs_file_open, spack_rfs_file_close,
            spack_rfs_file_read, spack_rfs_file_write, spack_rfs_file_flush,
            spack_rfs_file_seek, spack_rfs_file_tell, spack_rfs_file_eof,
            spack_rfs_file_size,
            spack_rfs_file_size_by_path,
            spack_rfs_file_exist,
            spack_rfs_file_rm,
            /*dir*/
            sizeof(struct spack_rfs_dir), spack_rfs_dir_open, spack_rfs_dir_close, spack_rfs_dir_read,
            spack_rfs_dir_exist,
            spack_rfs_dir_rm,
            spack_rfs_dir_mk,
            spack_rfs_dir_mk_recursion);
    if (backend == NULL) {
        mem_free(alloc, rfs_backend);
        return NULL;
    }

    return backend;
}
