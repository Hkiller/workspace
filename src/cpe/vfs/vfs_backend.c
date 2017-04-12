#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "vfs_backend_i.h"
#include "vfs_mount_point_i.h"

vfs_backend_t
vfs_backend_create(
    vfs_mgr_t mgr, const char * name, void * ctx,
    /*env*/
    vfs_backend_env_clear_fun_t env_clear,
    /*file*/
    uint32_t file_capacity, vfs_file_open_fun_t file_open, vfs_file_close_fun_t file_close,
    vfs_file_read_fun_t file_read, vfs_file_write_fun_t file_write, vfs_file_flush_fun_t file_flush,
    vfs_file_seek_fun_t file_seek, vfs_file_tell_fun_t file_tell, vfs_file_eof_fun_t file_eof_p,
    vfs_file_size_fun_t file_size,
    vfs_file_size_by_path_fun_t file_size_by_path,
    vfs_file_exist_fun_t file_exist,
    vfs_file_rm_fun_t file_rm,
    /*dir*/
    uint32_t dir_capacity, vfs_dir_open_fun_t dir_open, vfs_dir_close_fun_t dir_close,
    vfs_dir_read_fun_t dir_read,
    vfs_dir_exist_fun_t dir_exist,
    vfs_dir_rm_fun_t dir_rm,
    vfs_dir_mk_fun_t dir_mk,
    vfs_dir_mk_fun_t dir_mk_recursion)
{
    vfs_backend_t backend;

    if (vfs_backend_find_by_name(mgr, name) != NULL) {
        CPE_ERROR(mgr->m_em, "vfs_backend_create: name %s duplicate!", name);
        return NULL;
    }
    
    backend = mem_alloc(mgr->m_alloc, sizeof(struct vfs_backend));
    if (backend == NULL) {
        CPE_ERROR(mgr->m_em, "vfs_backend_create: alloc fail!");
        return NULL;
    }

    backend->m_mgr = mgr;
    cpe_str_dup(backend->m_name, sizeof(backend->m_name), name);
    TAILQ_INIT(&backend->m_mount_points);
    backend->m_ctx = ctx;
    backend->m_env_clear = env_clear;
    backend->m_file_capacity = file_capacity;
    backend->m_file_open = file_open;
    backend->m_file_close = file_close;
    backend->m_file_read = file_read;
    backend->m_file_write = file_write;
    backend->m_file_flush = file_flush;
    backend->m_file_seek = file_seek;
    backend->m_file_tell = file_tell;
    backend->m_file_eof = file_eof_p;
    backend->m_file_size = file_size;
    backend->m_file_size_by_path = file_size_by_path;
    backend->m_file_exist = file_exist;
    backend->m_file_rm = file_rm;
    backend->m_dir_capacity = dir_capacity;
    backend->m_dir_open = dir_open;
    backend->m_dir_close = dir_close;
    backend->m_dir_read = dir_read;
    backend->m_dir_exist = dir_exist;
    backend->m_dir_rm = dir_rm;
    backend->m_dir_mk = dir_mk;
    backend->m_dir_mk_recursion = dir_mk_recursion;

    TAILQ_INSERT_TAIL(&mgr->m_backends, backend, m_next);
    
    return backend;
}

void vfs_backend_free(vfs_backend_t backend) {
    vfs_mgr_t mgr = backend->m_mgr;

    while(!TAILQ_EMPTY(&backend->m_mount_points)) {
        vfs_mount_point_free(TAILQ_FIRST(&backend->m_mount_points));
    }
    
    TAILQ_REMOVE(&mgr->m_backends, backend, m_next);
    mem_free(mgr->m_alloc, backend);
}
    
vfs_backend_t vfs_backend_find_by_name(vfs_mgr_t mgr, const char * name) {
    vfs_backend_t backend;

    TAILQ_FOREACH(backend, &mgr->m_backends, m_next) {
        if (strcmp(backend->m_name, name) == 0) return backend;
    }

    return NULL;
}

vfs_backend_t vfs_backend_native(vfs_mgr_t mgr) {
    return TAILQ_FIRST(&mgr->m_backends);
}

void * vfs_backend_ctx(vfs_backend_t backend) {
    return backend->m_ctx;
}
