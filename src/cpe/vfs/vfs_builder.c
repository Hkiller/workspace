#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/vfs/vfs_mount_point.h"
#include "cpe/vfs/vfs_backend.h"
#include "vfs_builder_i.h"
#include "vfs_builder_entry_i.h"
#include "vfs_builder_backend_i.h"

vfs_builder_t vfs_builder_create(vfs_mgr_t mgr, const char * path) {
    vfs_backend_t backend;
    vfs_builder_backend_t builder_backend;
    vfs_builder_t builder = NULL;
    vfs_mount_point_t from;
    
    backend = vfs_backend_find_by_name(mgr, "builder:");
    if (backend == NULL) {
        backend = vfs_builder_create_backend(mgr);
        if (backend == NULL) return NULL; 
    }
    builder_backend = vfs_backend_ctx(backend);
    
    builder = mem_alloc(mgr->m_alloc, sizeof(struct vfs_builder));
    if (builder == NULL) {
        CPE_ERROR(mgr->m_em, "vfs_builder_create: alloc fail!");
        goto CREATE_ERROR;
    }

    builder->m_mgr = mgr;
    builder->m_backend = builder_backend;
    builder->m_root = NULL;
    builder->m_mount_point = NULL;
    
    builder->m_root = vfs_builder_entry_create(builder, NULL, path, path + strlen(path), 1);
    if (builder->m_root == NULL) {
        CPE_ERROR(mgr->m_em, "vfs_builder_create: create root entry fail!");
        goto CREATE_ERROR;
    }

    if (path[0] == '/') {
        from = vfs_mgr_root_point(mgr);
        path++;
    }
    else {
        from = vfs_mgr_current_point(mgr);
    }
    
    builder->m_mount_point = vfs_mount_point_mount(from, path, builder, backend);
    if (builder->m_mount_point == NULL) {
        CPE_ERROR(mgr->m_em, "vfs_builder_create: mount fail!");
        goto CREATE_ERROR;
    }

    TAILQ_INSERT_TAIL(&builder_backend->m_builders, builder, m_next);

    return builder;
    
CREATE_ERROR:
    if (builder) {
        if (builder->m_mount_point) {
            vfs_mount_point_unmount(mgr->m_root, builder->m_root->m_name);
            builder->m_mount_point = NULL;
        }
        
        if (builder->m_root) {
            vfs_builder_entry_free(builder->m_root);
            builder->m_root = NULL;
        }
        
        mem_free(mgr->m_alloc, builder);
    }

    if (TAILQ_EMPTY(&builder_backend->m_builders)) {
        vfs_backend_free(backend);
    }
    
    return NULL;
}

void vfs_builder_free(vfs_builder_t builder) {
    if (builder->m_mount_point) {
        vfs_mount_point_unmount(vfs_mgr_root_point(builder->m_mgr), builder->m_root->m_name);
        builder->m_mount_point = NULL;
    }
        
    if (builder->m_root) {
        vfs_builder_entry_free(builder->m_root);
        builder->m_root = NULL;
    }

    assert(builder->m_backend);
    TAILQ_REMOVE(&builder->m_backend->m_builders, builder, m_next);

    if (TAILQ_EMPTY(&builder->m_backend->m_builders)) {
        vfs_backend_t backend = vfs_backend_find_by_name(builder->m_mgr, "builder:");
        assert(backend);
        vfs_backend_free(backend);
        mem_free(builder->m_mgr->m_alloc, builder->m_backend);
    }
    
    mem_free(builder->m_mgr->m_alloc, builder);
}

const char * vfs_builder_path(vfs_builder_t builder) {
    return builder->m_root->m_name;
}

