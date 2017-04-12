#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/vfs/vfs_backend.h"
#include "vfs_manage_i.h"
#include "vfs_mount_point_i.h"
#include "vfs_backend_i.h"

vfs_mgr_t
vfs_mgr_create(mem_allocrator_t alloc, error_monitor_t em) {
    vfs_mgr_t mgr;
    char * root_backend_env;
    const char * pname;
    
    root_backend_env = cpe_str_mem_dup(alloc, "");
    if (root_backend_env == NULL) {
        CPE_ERROR(em, "vfs_mgr_create: dup root backend env fail!");
        return NULL;
    }
    
    mgr = mem_alloc(alloc, sizeof(struct vfs_mgr));
    if (mgr == NULL) {
        CPE_ERROR(em, "vfs_mgr_create: alloc fail!");
        mem_free(alloc, root_backend_env);
        return NULL;
    }

    mgr->m_alloc = alloc;
    mgr->m_em = em;
    mgr->m_root = NULL;
    mgr->m_current = NULL;

    TAILQ_INIT(&mgr->m_backends);
    TAILQ_INIT(&mgr->m_free_mount_points);
    
    if (vfs_backend_native_create(mgr) != 0) {
        vfs_mgr_free(mgr);
        return NULL;
    }

    mem_buffer_init(&mgr->m_search_path_buffer, mgr->m_alloc);
    mem_buffer_init(&mgr->m_tmp_buffer, mgr->m_alloc);

    pname = "root:";
    mgr->m_root = vfs_mount_point_create(mgr, pname, pname + strlen(pname), NULL, root_backend_env, TAILQ_FIRST(&mgr->m_backends));
    if (mgr->m_root == NULL) {
        mem_free(alloc, root_backend_env);
        vfs_mgr_free(mgr);
        return NULL;
    }

    pname = "curent:";
    mgr->m_current = vfs_mount_point_create(mgr, pname, pname + strlen(pname), NULL, NULL, TAILQ_FIRST(&mgr->m_backends));
    if (mgr->m_root == NULL) {
        vfs_mgr_free(mgr);
        return NULL;
    }
    
    return mgr;
}

void vfs_mgr_free(vfs_mgr_t mgr) {
    while(!TAILQ_EMPTY(&mgr->m_backends)) {
        vfs_backend_free(TAILQ_FIRST(&mgr->m_backends));
    }
    assert(mgr->m_root == NULL);
    assert(mgr->m_current == NULL);

    while(!TAILQ_EMPTY(&mgr->m_free_mount_points)) {
        vfs_mount_point_real_free(TAILQ_FIRST(&mgr->m_free_mount_points));
    }

    mem_buffer_clear(&mgr->m_search_path_buffer);
    mem_buffer_clear(&mgr->m_tmp_buffer);
    
    mem_free(mgr->m_alloc, mgr);
}

vfs_mount_point_t vfs_mgr_root_point(vfs_mgr_t mgr) {
    return mgr->m_root;
}

vfs_mount_point_t vfs_mgr_current_point(vfs_mgr_t mgr) {
    return mgr->m_current;
}
