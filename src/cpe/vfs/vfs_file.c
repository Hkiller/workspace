#include <assert.h>
#include "cpe/utils/buffer.h"
#include "vfs_file_i.h"
#include "vfs_backend_i.h"
#include "vfs_mount_point_i.h"

vfs_file_t vfs_file_open(vfs_mgr_t mgr, const char * input_path, const char * mod) {
    vfs_file_t f;
    vfs_mount_point_t mount_point;
    vfs_backend_t backend;
    const char * path = input_path;

    mount_point = vfs_mount_point_find_by_path(mgr, &path);
    if (mount_point == NULL) {
        CPE_ERROR(mgr->m_em, "vfs_file_open: mount point of path %s not exist!", input_path);
        return NULL;
    }

    backend = mount_point->m_backend;
    assert(backend);
    
    if (backend->m_file_open == NULL) {
        CPE_ERROR(mgr->m_em, "vfs_file_open: backend %s not support open file!", backend->m_name);
        return NULL;
    }
    
    f = mem_alloc(mgr->m_alloc, sizeof(struct vfs_file) + backend->m_file_capacity);
    if (f == NULL) {
        CPE_ERROR(mgr->m_em, "vfs_file_open: alloc fail!");
        return NULL;
    }

    f->m_mgr = mgr;
    f->m_backend = backend;
    
    if (backend->m_file_open(backend->m_ctx, mount_point->m_backend_env, f, path, mod) != 0) {
        mem_free(mgr->m_alloc, f);
        return NULL;
    }

    return f;
}

void vfs_file_close(vfs_file_t f) {
    vfs_mgr_t mgr = f->m_mgr;
    vfs_backend_t backend = f->m_backend;

    backend->m_file_close(backend->m_ctx, f);

    mem_free(mgr->m_alloc, f);
}

uint8_t vfs_file_exist(vfs_mgr_t mgr, const char * path) {
    vfs_mount_point_t mount_point;
    vfs_backend_t backend;

    mount_point = vfs_mount_point_find_by_path(mgr, &path);
    if (mount_point == NULL) return 0;

    if (path[0] == 0) return 0;

    backend = mount_point->m_backend;
    assert(backend);

    return backend->m_file_exist ? backend->m_file_exist(backend->m_ctx, mount_point->m_backend_env, path) : 0;
}

int vfs_file_rm(vfs_mgr_t mgr, const char * path) {
    vfs_mount_point_t mount_point;
    vfs_backend_t backend;

    mount_point = vfs_mount_point_find_by_path(mgr, &path);
    if (mount_point == NULL) return -1;
    
    backend = mount_point->m_backend;
    assert(backend);

    return backend->m_file_rm ? backend->m_file_rm(backend->m_ctx, mount_point->m_backend_env, path) : -1;
}

void * vfs_file_data(vfs_file_t f) {
    return f + 1;
}

ssize_t vfs_file_size(vfs_file_t f) {
    return f->m_backend->m_file_size
        ? f->m_backend->m_file_size(f->m_backend->m_ctx, f)
        : -1;
}

ssize_t vfs_file_size_by_path(vfs_mgr_t mgr, const char * path) {
    vfs_mount_point_t mount_point;
    vfs_backend_t backend;

    mount_point = vfs_mount_point_find_by_path(mgr, &path);
    if (mount_point == NULL) return -1;
    
    backend = mount_point->m_backend;
    assert(backend);

    return backend->m_file_size_by_path
        ? backend->m_file_size_by_path(backend->m_ctx, mount_point->m_backend_env, path)
        : -1;
}

ssize_t vfs_file_read(vfs_file_t f, void * buf, size_t size) {
    return f->m_backend->m_file_read
        ? f->m_backend->m_file_read(f->m_backend->m_ctx, f, buf, size)
        : -1;
}

ssize_t vfs_file_write(vfs_file_t f, void const * buf, size_t size) {
    return f->m_backend->m_file_write
        ? f->m_backend->m_file_write(f->m_backend->m_ctx, f, buf, size)
        : -1;
}

int vfs_file_flush(vfs_file_t f) {
    return f->m_backend->m_file_flush
        ? f->m_backend->m_file_flush(f->m_backend->m_ctx, f)
        : 0;
}

int vfs_file_seek(vfs_file_t f, ssize_t off, vfs_file_seek_op_t op) {
    return f->m_backend->m_file_seek
        ? f->m_backend->m_file_seek(f->m_backend->m_ctx, f, off, op)
        : -1;
}

ssize_t vfs_file_tell(vfs_file_t f) {
    return f->m_backend->m_file_tell
        ? f->m_backend->m_file_tell(f->m_backend->m_ctx, f)
        : -1;
}

uint8_t vfs_file_eof(vfs_file_t f) {
    return f->m_backend->m_file_eof
        ? f->m_backend->m_file_eof(f->m_backend->m_ctx, f)
        : 1;
}

ssize_t vfs_file_load_to_buffer_by_path(mem_buffer_t buffer, vfs_mgr_t mgr, const char * path) {
    vfs_file_t f = vfs_file_open(mgr, path, "rb");
    
    if (f) {
        ssize_t r = vfs_file_load_to_buffer(buffer, f);
        vfs_file_close(f);
        return r;
    }
    else {
        return -1;
    }
}

ssize_t vfs_file_load_to_buffer(mem_buffer_t buffer, vfs_file_t f) {
    vfs_mgr_t mgr = f->m_mgr;
    vfs_backend_t backend = f->m_backend;
    
    if (backend->m_file_size) {
        ssize_t rv = backend->m_file_size(backend->m_ctx, f);

        if (rv > 0) {
            void * buf = mem_buffer_alloc(buffer, rv);
            if (buf == NULL) {
                CPE_ERROR(mgr->m_em, "vfs_file_load_to_buffer: alloc %d from buffer fail!", (int)rv);
                return -1;
            }

            return backend->m_file_read(backend->m_ctx, f, buf, rv);
        }
        else {
            return rv;
        }
    }
    else {
        ssize_t total_size;

        total_size = 0;
        do {
            size_t buf_size;
            char * buf;
            ssize_t rv;

            buf_size = buffer->m_auto_inc_size;
            buf = mem_buffer_alloc(buffer, buf_size);
            if (buf == NULL) {
                CPE_ERROR(mgr->m_em, "vfs_file_load_to_buffer: alloc %d from buffer fail!", (int)buf_size);
                return -1;
            }
            
            rv = backend->m_file_read(backend->m_ctx, f, buf, buf_size);
            if (rv < 0) return rv;
            if (rv == 0) break;
            
            total_size += rv;
        } while(1);

        return total_size;
    }
}

ssize_t vfs_file_write_from_buffer(vfs_file_t f, mem_buffer_t buffer) {
    ssize_t total_size;
    struct mem_buffer_trunk * trunk;

    total_size = 0;

    for(trunk = mem_buffer_trunk_first(buffer);
        trunk;
        trunk = mem_buffer_trunk_next(trunk))
    {
        ssize_t write_size;
        char * buf = mem_trunk_data(trunk);
        size_t size = mem_trunk_size(trunk);

        if (size <= 0) continue;
        
        write_size = vfs_file_write(f, buf, size);
        if (write_size < 0) {
            return -1;
        }

        assert(write_size == size);
        total_size += write_size;
    }

    return total_size;
}

int vfs_file_copy(vfs_mgr_t vfs, const char * output, const char * input) {
    ssize_t totalSize;
    size_t writeSize;
    size_t writeOkSize;
    size_t size;
    vfs_file_t fp_i;
    vfs_file_t fp_o;
    char buf[512];

    fp_i = vfs_file_open(vfs, input, "rb");
    if (fp_i == NULL) return -1;

    fp_o = vfs_file_open(vfs, output, "wb");
    if (fp_o == NULL) {
        vfs_file_close(fp_i);
        return -1;
    }

    totalSize = 0;
    while((size = vfs_file_read(fp_i, buf, sizeof(buf))) > 0) {
        writeOkSize = 0;
        while(size > writeOkSize
              && (writeSize = vfs_file_write(fp_o, buf + writeOkSize, size - writeOkSize)) > 0)
        {
            writeOkSize += writeSize;
        }

        totalSize += writeOkSize;

        if (writeOkSize < size) break;
    }

    /* if (ferror(fp_o)) { */
    /*     totalSize = -1; */
    /* } */

    vfs_file_close(fp_i);
    vfs_file_close(fp_o);
    
    return (int)totalSize;
}
