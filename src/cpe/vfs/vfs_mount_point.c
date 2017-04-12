#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "vfs_mount_point_i.h"
#include "vfs_backend_i.h"

vfs_mount_point_t
vfs_mount_point_create(vfs_mgr_t mgr, const char * name, const char * name_end, vfs_mount_point_t parent, void * backend_env, vfs_backend_t backend) {
    vfs_mount_point_t mount_point;
    size_t name_len = name_end - name;

    if (name_len + 1 > CPE_TYPE_ARRAY_SIZE(struct vfs_mount_point, m_name)) {
        CPE_ERROR(mgr->m_em, "vfs_mount_point_create: name %s overflow!", name);
        return NULL;
    }
        
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
    memcpy(mount_point->m_name, name, name_len);
    mount_point->m_name[name_len] = 0;
    mount_point->m_name_len = name_len;
    TAILQ_INIT(&mount_point->m_childs);

    mount_point->m_bridger_to = NULL;
    TAILQ_INIT(&mount_point->m_bridger_froms);
    
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

    vfs_mount_point_set_bridger_to(mount_point, NULL);
    
    while(!TAILQ_EMPTY(&mount_point->m_bridger_froms)) {
        vfs_mount_point_set_bridger_to(TAILQ_FIRST(&mount_point->m_bridger_froms), NULL);
    }

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

int vfs_mount_point_set_bridger_to(vfs_mount_point_t mp, vfs_mount_point_t to) {
    if (to != mp->m_bridger_to) {
        if (mp->m_bridger_to) {
            TAILQ_REMOVE(&mp->m_bridger_to->m_bridger_froms, mp, m_next_bridger_to);
        }

        mp->m_bridger_to = to;
        
        if (mp->m_bridger_to) {
            TAILQ_INSERT_TAIL(&mp->m_bridger_to->m_bridger_froms, mp, m_next_bridger_to);
        }
    }
    
    return 0;
}

int vfs_mount_point_set_bridger_to_by_path(vfs_mount_point_t mp, const char * path) {
    vfs_mount_point_t to = NULL;

    if (path) {
        to = vfs_mount_point_find_by_path(mp->m_mgr, &path);
        if (to == NULL) {
            CPE_ERROR(mp->m_mgr->m_em, "vfs_mount_point_set_bridger_to: find target mount point %s fail!", path);
            return -1;
        }

        if (path[0]) {
            to = vfs_mount_point_mount(to, path, NULL, NULL);
            if (to == NULL) {
                CPE_ERROR(mp->m_mgr->m_em, "vfs_mount_point_set_bridger_to: create target mount point %s fail!", path);
                return -1;
            }
        }
    }

    return vfs_mount_point_set_bridger_to(mp, to);
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

static int vfs_mount_point_find_child_process_mount(vfs_mount_point_t * i_mp, const char ** path, uint8_t * path_in_tmp) {
    vfs_mount_point_t mp = *i_mp;
    while(mp->m_bridger_to) {
        size_t append_path_len;
        vfs_mount_point_t bridger_to = mp->m_bridger_to;
        vfs_mount_point_t next_mp;

        /*当前点走到最后一个转接 */
        while(bridger_to->m_bridger_to) bridger_to = bridger_to->m_bridger_to;

        /*向上搜索， 寻找到拥有backend的挂节点或者新的挂节点 */
        next_mp = bridger_to;
        append_path_len = 0;
        while(next_mp && next_mp->m_backend == NULL && next_mp->m_bridger_to == NULL) {
            append_path_len += next_mp->m_name_len + 1;
            next_mp = next_mp->m_parent;
        }

        /*没有找到则错误返回 */
        if (next_mp == NULL) {
            CPE_ERROR(mp->m_mgr->m_em, "vfs_mount_point_find_child_process_mount: can`t found bridge to point with backend!");
            return -1;
        }

        /* 将新增的路径拼接到path中去 */
        if (append_path_len > 0) {
            char * insert_buf;
            vfs_mount_point_t tmp_mp;
            
            if (*path_in_tmp) {
                struct mem_buffer_pos pt;
                mem_buffer_begin(&pt, &mp->m_mgr->m_search_path_buffer);
                insert_buf = mem_pos_insert_alloc(&pt, append_path_len);
                if (insert_buf == NULL) {
                    CPE_ERROR(mp->m_mgr->m_em, "vfs_mount_point_find_child_process_mount: append path, insert alloc fail!");
                    return -1;
                }
            }
            else {
                size_t path_len = strlen(*path) + 1;

                mem_buffer_clear_data(&mp->m_mgr->m_search_path_buffer);
                
                insert_buf = mem_buffer_alloc(&mp->m_mgr->m_search_path_buffer, append_path_len + path_len);
                if (insert_buf == NULL) {
                    CPE_ERROR(mp->m_mgr->m_em, "vfs_mount_point_find_child_process_mount: append path, init alloc fail!");
                    return -1;
                }

                memcpy(insert_buf + append_path_len, *path, path_len);
                *path_in_tmp = 1;
            }

            for(tmp_mp = bridger_to; tmp_mp != next_mp; tmp_mp = tmp_mp->m_parent) {
                assert(append_path_len >= (tmp_mp->m_name_len + 1));

                append_path_len -= (tmp_mp->m_name_len + 1);
                memcpy(insert_buf + append_path_len, tmp_mp->m_name, tmp_mp->m_name_len);
                insert_buf[append_path_len + tmp_mp->m_name_len] = '/';
            }

            *path = mem_buffer_make_continuous(&mp->m_mgr->m_search_path_buffer, 0);
            //printf("xxxxx: path=%s\n", *path);
        }
        
        if (next_mp->m_backend) {
            *i_mp = next_mp;
            break;
        }

        mp = next_mp;
        assert(mp->m_bridger_to);
    }

    return 0;
}
    
vfs_mount_point_t vfs_mount_point_find_child_by_path(vfs_mount_point_t mount_point, const char * * inout_path) {
    const char * sep;
    const char * path = *inout_path;
    uint8_t path_in_tmp = 0;
    const char * last_found_path = path;
    vfs_mount_point_t last_found = mount_point->m_backend ? mount_point : NULL;

    if (last_found && vfs_mount_point_find_child_process_mount(&last_found, &path, &path_in_tmp) != 0) return NULL;
                                                               
    while((sep = strchr(path, '/'))) {
        if (sep > path) {
            mount_point = vfs_mount_point_child_find_by_name_ex(mount_point, path, sep);
            if (mount_point == NULL) break;

            path = sep + 1;

            if (vfs_mount_point_find_child_process_mount(&mount_point, &path, &path_in_tmp) != 0) return NULL;
            
            if (mount_point->m_backend) {
                last_found = mount_point;
                last_found_path = path;
            }
        }
        else {
            path = sep + 1;
            if (mount_point == last_found) last_found_path = path;
        }
    }
    
    if (mount_point && path[0]) {
        size_t path_len = strlen(path);
        mount_point = vfs_mount_point_child_find_by_name_ex(mount_point, path, path + path_len);
        if (mount_point) {
            path += path_len;

            if (vfs_mount_point_find_child_process_mount(&mount_point, &path, &path_in_tmp) != 0) return NULL;
            
            if (mount_point->m_backend) {
                last_found = mount_point;
                last_found_path = path;
            }
        }
    }

    *inout_path = last_found_path;
    return last_found;
}

void vfs_mount_point_set_backend(vfs_mount_point_t mp, void * backend_env, vfs_backend_t backend) {
    if (mp->m_backend) {
        if (mp->m_backend->m_env_clear && mp->m_backend_env) {
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
    vfs_mount_point_t mp = from;

    while((sep = strchr(path, '/'))) {
        if (sep > path) {
            vfs_mount_point_t child_mp = NULL;
            child_mp = vfs_mount_point_child_find_by_name_ex(mp, path, sep);
            if (child_mp == NULL) {
                child_mp = vfs_mount_point_create(mgr, path, sep, mp, NULL, NULL);
                if (child_mp == NULL) {
                    CPE_ERROR(mgr->m_em, "vfs_mount_point_mount: create mount point %s fail!", path);
                    return NULL;
                }
            }

            mp = child_mp;
        }
        
        path = sep + 1;
    }

    if (path[0]) {
        mp = vfs_mount_point_create(mgr, path, path + strlen(path), mp, NULL, NULL);
        if (mp == NULL) {
            CPE_ERROR(mgr->m_em, "vfs_mount_point_mount: create mount point %s fail!", path);
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

    mp = vfs_mount_point_child_find_by_path_ex(from, path, path + strlen(path));
    if (mp == NULL) {
        CPE_ERROR(mgr->m_em, "vfs_mount_point_unmount: point %s not exist!", path);
        return -1;
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

vfs_mount_point_t
vfs_mount_point_child_find_by_name_ex(vfs_mount_point_t parent, const char * name, const char * name_end) {
    vfs_mount_point_t child_mp = NULL;
        
    TAILQ_FOREACH(child_mp, &parent->m_childs, m_next_for_parent) {
        if (name_end - name == child_mp->m_name_len && memcmp(child_mp->m_name, name, child_mp->m_name_len) == 0) {
            return child_mp;
        }
    }

    return NULL;
}

vfs_mount_point_t
vfs_mount_point_child_find_by_path_ex(vfs_mount_point_t parent, const char * path, const char * path_end) {
    const char * sep;
    
    while((sep = memchr(path, '/', path_end - path))) {
        if (sep > path) {
            parent = vfs_mount_point_child_find_by_name_ex(parent, path, sep);
            if (parent == NULL) return NULL;
        }
        path = sep + 1;
    }

    if (path[0]) {
        parent = vfs_mount_point_child_find_by_name_ex(parent, path, path_end);
    }
    
    return parent;
}
