#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "vfs_mount_point_i.h"
#include "vfs_backend_i.h"

vfs_mount_point_t
vfs_mount_point_create(vfs_mgr_t mgr, const char * name, vfs_mount_point_t parent, void * backend_env, vfs_backend_t backend) {
    vfs_mount_point_t mount_point;

    mount_point = TAILQ_FIRST(&mgr->m_free_mount_points);
    if (mount_point) {
        TAILQ_REMOVE(&mgr->m_free_mount_points, mount_point, m_next_for_backend);
    }
    else {
        mount_point = mem_alloc(mgr->m_alloc, sizeof(struct vfs_mount_point));
        if (mount_point == NULL) {
            CPE_ERROR(mgr->m_em, "vfs_mount_point_create: alloc fail");
            return NULL;
        }
    }

    mount_point->m_mgr = mgr;
    cpe_str_dup(mount_point->m_name, sizeof(mount_point->m_name), name);
    TAILQ_INIT(&mount_point->m_childs);
    
    mount_point->m_parent = parent;
    if (mount_point->m_parent) {
        TAILQ_INSERT_TAIL(&mount_point->m_parent->m_childs, mount_point, m_next_for_parent);
    }

    mount_point->m_backend_env = backend_env;
    mount_point->m_backend = backend;
    if(mount_point->m_backend) {
        TAILQ_INSERT_TAIL(&backend->m_mount_points, mount_point, m_next_for_backend);
    }

    return mount_point;
}

void vfs_mount_point_free(vfs_mount_point_t mount_point) {
    vfs_mgr_t mgr = mount_point->m_mgr;

    while(!TAILQ_EMPTY(&mount_point->m_childs)) {
        vfs_mount_point_free(TAILQ_FIRST(&mount_point->m_childs));
    }
    
    if (mount_point->m_mgr->m_root == mount_point) {
        mount_point->m_mgr->m_root = NULL;
    }

    if (mount_point->m_mgr->m_current == mount_point) {
        mount_point->m_mgr->m_current = NULL;
    }
    
    if (mount_point->m_parent) {
        TAILQ_REMOVE(&mount_point->m_parent->m_childs, mount_point, m_next_for_parent);
    }

    if (mount_point->m_backend) {
        if (mount_point->m_backend_env) {
            mount_point->m_backend->m_env_clear(mount_point->m_backend->m_ctx, mount_point->m_backend_env);
            mount_point->m_backend_env = NULL;
        }
        
        TAILQ_REMOVE(&mount_point->m_backend->m_mount_points, mount_point, m_next_for_backend);
    }

    TAILQ_INSERT_TAIL(&mgr->m_free_mount_points, mount_point, m_next_for_backend);
}

void vfs_mount_point_real_free(vfs_mount_point_t mount_point) {
    TAILQ_REMOVE(&mount_point->m_mgr->m_free_mount_points, mount_point, m_next_for_backend);
    mem_free(mount_point->m_mgr->m_alloc, mount_point);
}

vfs_mount_point_t vfs_mount_point_find_by_path(vfs_mgr_t mgr, const char * * path) {
    if ((*path)[0] == '/') {
        (*path)++;
        return vfs_mount_point_find_child_by_path(mgr->m_root, path);
    }
    else {
        return mgr->m_current ? vfs_mount_point_find_child_by_path(mgr->m_current, path) : NULL;
    }
}

vfs_mount_point_t vfs_mount_point_find_child_by_name(vfs_mount_point_t p, const char * name) {
    vfs_mount_point_t sub_point;
    
    TAILQ_FOREACH(sub_point, &p->m_childs, m_next_for_parent) {
        if (strcmp(sub_point->m_name, name) == 0) return sub_point;
    }

    return NULL;
}

vfs_mount_point_t vfs_mount_point_find_child_by_path(vfs_mount_point_t mount_point, const char * * path) {
    const char * sep;
    vfs_mount_point_t sub_point;
    char buf[32];
    char * sub_name;

CHECK_AGAIN:    
    if (TAILQ_EMPTY(&mount_point->m_childs)) return mount_point;

    sep = strchr(*path, '/');
    if (sep == NULL) return mount_point;

    sub_name = cpe_str_dup_range(buf, sizeof(buf), *path, sep);
    if (sub_name == NULL) {
        CPE_ERROR(mount_point->m_mgr->m_em, "vfs_mount_point_find_child_by_path: node name len overflow!");
        return NULL;
    }
    
    TAILQ_FOREACH(sub_point, &mount_point->m_childs, m_next_for_parent) {
        if (strcmp(sub_point->m_name, sub_name) == 0) {
            *path = sep + 1;
            mount_point = sub_point;
            goto CHECK_AGAIN;
        }
    }

    return mount_point;
}

void vfs_mount_point_set_backend(vfs_mount_point_t mp, void * backend_env, vfs_backend_t backend) {
    if (mp->m_backend) {
        if (mp->m_backend_env) {
            mp->m_backend->m_env_clear(mp->m_backend->m_ctx, mp->m_backend_env);
        }
        
        TAILQ_REMOVE(&mp->m_backend->m_mount_points, mp, m_next_for_backend);
    }

    mp->m_backend_env = backend_env;
    mp->m_backend = backend;
    
    if (mp->m_backend) {
        TAILQ_INSERT_TAIL(&mp->m_backend->m_mount_points, mp, m_next_for_backend);
    }
}

void vfs_mount_point_clear_path(vfs_mount_point_t mount_point) {
    while(TAILQ_EMPTY(&mount_point->m_childs) && mount_point->m_backend == NULL) {
        vfs_mount_point_t p = mount_point->m_parent;
        if (p == NULL) break;
        vfs_mount_point_free(mount_point);
        mount_point = p;
    }
}

vfs_mount_point_t
vfs_mount_point_mount(vfs_mount_point_t from, const char * path, void * backend_env, vfs_backend_t backend) {
    vfs_mgr_t mgr = from->m_mgr;
    const char * sep;
    vfs_mount_point_t sub_point;
    char buf[32];
    char * sub_name;
    vfs_mount_point_t mp = from;

    /*寻找已经有的部分 */
    while (!TAILQ_EMPTY(&mp->m_childs)) {
        sep = strchr(path, '/');
        if (sep == NULL) break;

        sub_name = cpe_str_dup_range(buf, sizeof(buf), path, sep);
        if (sub_name == NULL) {
            CPE_ERROR(mgr->m_em, "vfs_mount_point_mount: node name len overflow!");
            return NULL;
        }

        TAILQ_FOREACH(sub_point, &mp->m_childs, m_next_for_parent) {
            if (strcmp(sub_point->m_name, sub_name) == 0) {
                path = sep + 1;
                mp = sub_point;
                break;
            }
        }

        if (sub_point != mp) break;
    }

    while((sep = strchr(path, '/'))) {
        sub_name = cpe_str_dup_range(buf, sizeof(buf), path, sep);
        if (sub_name == NULL) {
            CPE_ERROR(mgr->m_em, "vfs_mount_point_mount: node name len overflow!");
            return NULL;
        }

        mp = vfs_mount_point_create(mgr, sub_name, mp, NULL, NULL);
        if (mp == NULL) {
            CPE_ERROR(mgr->m_em, "vfs_mount_point_mount: create sub node %s fail!", sub_name);
            return NULL;
        }
    }

    if (path[0]) {
        mp = vfs_mount_point_create(mgr, sub_name, mp, NULL, NULL);
        if (mp == NULL) {
            CPE_ERROR(mgr->m_em, "vfs_mount_point_mount: create sub node %s fail!", path);
            return NULL;
        }
    }

    assert(mp);
    vfs_mount_point_set_backend(mp, backend_env, backend);
    
    return mp;
}

int vfs_mount_point_unmount(vfs_mount_point_t from, const char * path) {
    vfs_mgr_t mgr = from->m_mgr;
    vfs_mount_point_t mp = from;
    vfs_mount_point_t sub_point;

    /*寻找已经有的部分 */
    while (!TAILQ_EMPTY(&mp->m_childs)) {
        const char * sep;
        char buf[32];
        char * sub_name;
        
        sep = strchr(path, '/');
        if (sep == NULL) break;

        sub_name = cpe_str_dup_range(buf, sizeof(buf), path, sep);
        if (sub_name == NULL) {
            CPE_ERROR(mgr->m_em, "vfs_mount_point_unmount: node name len overflow!");
            return -1;
        }

        sub_point = vfs_mount_point_find_child_by_name(mp, sub_name);
        if (sub_point == NULL) {
            CPE_ERROR(mgr->m_em, "vfs_mount_point_unmount: point %s not exist!", sub_name);
            return -1;
        }
        
        path = sep + 1;
        mp = sub_point;
    }

    if (path[0]) {
        sub_point = vfs_mount_point_find_child_by_name(mp, path);
        if (sub_point == NULL) {
            CPE_ERROR(mgr->m_em, "vfs_mount_point_unmount: point %s not exist!", path);
            return -1;
        }
        
        mp = sub_point;
    }

    if (mp->m_backend == NULL) {
        CPE_ERROR(mgr->m_em, "vfs_mount_point_unmount: point %s no explicit backend!", mp->m_name);
        return -1;
    }

    if (mp->m_parent == NULL) {
        vfs_backend_t native_backend = vfs_backend_native(mgr);
        
        if (mp->m_backend != native_backend) {
            if (mp == mgr->m_current) {
                vfs_mount_point_set_backend(mp, NULL, native_backend);
            }
            else if (mp == mgr->m_root) {
                char * d = cpe_str_mem_dup(mgr->m_alloc, "");
                if (d == NULL) {
                    CPE_ERROR(mgr->m_em, "vfs_mount_point_unmount: dup root fail!");
                    return -1;
                }
                
                vfs_mount_point_set_backend(mp, d, native_backend);
            }
            else {
                CPE_ERROR(mgr->m_em, "vfs_mount_point_unmount: root mount point %s type unknown!", mp->m_name);
                return -1;
            }
        }
    }
    else {
        vfs_mount_point_set_backend(mp, NULL, NULL);
        vfs_mount_point_clear_path(mp);
    }

    return 0;
}
