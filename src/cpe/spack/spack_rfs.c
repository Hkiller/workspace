#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/vfs/vfs_manage.h"
#include "cpe/vfs/vfs_mount_point.h"
#include "cpe/vfs/vfs_backend.h"
#include "spack_rfs_i.h"
#include "spack_rfs_backend_i.h"
#include "spack_rfs_entry_i.h"
#include "spack_proto_i.h"

spack_rfs_t spack_rfs_create(vfs_mgr_t vfs, const char * path, mem_allocrator_t alloc, error_monitor_t em) {
    vfs_backend_t backend;
    spack_rfs_backend_t rfs_backend;
    spack_rfs_t rfs = NULL;
    vfs_mount_point_t from;
    size_t path_len = strlen(path);
    
    backend = vfs_backend_find_by_name(vfs, "spack_rfs:");
    if (backend == NULL) {
        backend = spack_rfs_create_backend(vfs, alloc, em);
        if (backend == NULL) return NULL; 
    }
    rfs_backend = vfs_backend_ctx(backend);
    
    rfs = mem_alloc(alloc, sizeof(struct spack_rfs) + path_len + 1);
    if (rfs == NULL) {
        CPE_ERROR(em, "spack_rfs_create: alloc fail!");
        goto CREATE_ERROR;
    }

    memcpy(rfs + 1, path, path_len + 1);
    path = (const char *)(rfs + 1);
    
    rfs->m_vfs = vfs;
    rfs->m_alloc = alloc;
    rfs->m_em = em;
    rfs->m_backend = rfs_backend;
    rfs->m_root = NULL;
    rfs->m_mount_point = NULL;
    rfs->m_entry_count = 0;
    rfs->m_spack_size = 0;

    rfs->m_read_error = 0;
    
    rfs->m_data_capacity = 0;
    rfs->m_data_size = 0;
    rfs->m_data_own = 0;
    rfs->m_data = NULL;

    rfs->m_root = spack_rfs_entry_create(rfs, NULL, path, path + path_len, 1);
    if (rfs->m_root == NULL) {
        CPE_ERROR(em, "spack_rfs_create: create root entry fail!");
        goto CREATE_ERROR;
    }

    if (path[0] == '/') {
        from = vfs_mgr_root_point(vfs);
        path++;
    }
    else {
        from = vfs_mgr_current_point(vfs);
    }
    
    rfs->m_mount_point = vfs_mount_point_mount(from, path, rfs, backend);
    if (rfs->m_mount_point == NULL) {
        CPE_ERROR(em, "spack_rfs_create: mount fail!");
        goto CREATE_ERROR;
    }

    TAILQ_INSERT_TAIL(&rfs->m_backend->m_rfss, rfs, m_next);

    return rfs;
    
CREATE_ERROR:
    if (rfs) {
        if (rfs->m_mount_point) {
            vfs_mount_point_unmount(from, rfs->m_root->m_name);
            rfs->m_mount_point = NULL;
        }
        
        if (rfs->m_root) {
            spack_rfs_entry_free(rfs->m_root);
            rfs->m_root = NULL;
        }
        
        mem_free(alloc, rfs);
    }

    if (TAILQ_EMPTY(&rfs_backend->m_rfss)) {
        vfs_backend_free(backend);
    }
    
    return NULL;
}

void spack_rfs_free(spack_rfs_t rfs) {
    if (rfs->m_mount_point) {
        const char * path = rfs->m_root->m_name;
        vfs_mount_point_t from;

        if (path[0] == '/') {
            from = vfs_mgr_root_point(rfs->m_vfs);
            path++;
        }
        else {
            from = vfs_mgr_current_point(rfs->m_vfs);
        }
        
        vfs_mount_point_unmount(from, path);
        rfs->m_mount_point = NULL;
    }
        
    if (rfs->m_root) {
        spack_rfs_entry_free(rfs->m_root);
        rfs->m_root = NULL;
    }

    if (rfs->m_data && rfs->m_data_own) {
        mem_free(rfs->m_alloc, rfs->m_data);
        rfs->m_data_own = 0;
        rfs->m_data = NULL;
    }

    assert(rfs->m_backend);
    TAILQ_REMOVE(&rfs->m_backend->m_rfss, rfs, m_next);

    if (TAILQ_EMPTY(&rfs->m_backend->m_rfss)) {
        vfs_backend_t backend = vfs_backend_find_by_name(rfs->m_vfs, "spack_rfs:");
        assert(backend);
        vfs_backend_free(backend);
        mem_free(rfs->m_alloc, rfs->m_backend);
    }
    
    mem_free(rfs->m_alloc, rfs);
}

const char * spack_rfs_path(spack_rfs_t rfs) {
    return rfs->m_root->m_name;
}

void spack_rfs_clear(spack_rfs_t rfs) {
    while(!TAILQ_EMPTY(&rfs->m_root->m_dir.m_childs)) {
        spack_rfs_entry_free(TAILQ_FIRST(&rfs->m_root->m_dir.m_childs));
    }

    if (rfs->m_data && rfs->m_data_own) {
        mem_free(rfs->m_alloc, rfs->m_data);
    }

    rfs->m_read_error = 0;
    rfs->m_entry_count = 0;
    rfs->m_spack_size = 0;

    rfs->m_data_capacity = 0;
    rfs->m_data_size = 0;
    rfs->m_data_own = 0;
    rfs->m_data = NULL;
}

static int spack_rfs_build_entries(spack_rfs_t rfs) {
    struct spack_head const * head = rfs->m_data;
    struct spack_entry const * entry;
    uint32_t i;

    entry = (void const *)(head + 1);
    for(i = 0; i < rfs->m_entry_count; ++i, entry++) {
        uint32_t name_pos;
        uint32_t entry_data_start;
        uint32_t entry_data_size;

        CPE_COPY_NTOH32(&name_pos, &entry->m_name);
        CPE_COPY_NTOH32(&entry_data_start, &entry->m_data_start);
        CPE_COPY_NTOH32(&entry_data_size, &entry->m_data_size);

        if (name_pos > rfs->m_data_size) {
            CPE_ERROR(
                rfs->m_em, "spack_rfs_build_entries: entry %d name pos %d overflow, buf-size=%d",
                i, name_pos, (int)rfs->m_data_size);
            return -1;
        }

        if (entry_data_start + entry_data_size > rfs->m_data_size) {
            CPE_ERROR(
                rfs->m_em, "spack_rfs_build_entries: entry %d data (%d~%d) overflow, buf-size=%d",
                i, entry_data_size, entry_data_start + entry_data_size, (int)rfs->m_data_size);
            return -1;
        }

        if (spack_rfs_entry_file_create(rfs, ((const char *)rfs->m_data) + name_pos,  entry_data_start, entry_data_size) != 0) return -1;
    }
            
    return 0;
}

static void spack_rfs_read_total_size(spack_rfs_t rfs, void * data, size_t data_size) {
    if (rfs->m_entry_count == 0) {
        rfs->m_spack_size = sizeof(struct spack_head);
    }
    else {
        struct spack_entry entry_buf;
        struct spack_entry * last_entry;
        uint32_t last_entry_pos = sizeof(struct spack_head) + (rfs->m_entry_count - 1) * sizeof(struct spack_entry);
        uint32_t entry_start;
        uint32_t entry_size;
            
        if (last_entry_pos + sizeof(struct spack_entry) <= rfs->m_data_size) {
            /*数据在缓存中 */
            last_entry = (struct spack_entry *)(((char*)rfs->m_data) + last_entry_pos);
        }
        else if (last_entry_pos >= rfs->m_data_size) {
            /*数据在新输入的数据中 */
            last_entry = (struct spack_entry *)(((char*)data) + (last_entry_pos - rfs->m_data_size));
        }
        else {
            /*数据跨了新老缓存，则需要通过缓存拼接起来 */
            uint32_t buf_data_size = rfs->m_data_size - last_entry_pos;

            assert(buf_data_size > 0 && buf_data_size < sizeof(struct spack_entry));

            memcpy(&entry_buf, ((char*)rfs->m_data) + last_entry_pos, buf_data_size);
            memcpy(((char*)&entry_buf) + buf_data_size, data, sizeof(struct spack_entry) - buf_data_size);
            last_entry = &entry_buf;
        }

        CPE_COPY_NTOH32(&entry_start, &last_entry->m_data_start);
        CPE_COPY_NTOH32(&entry_size, &last_entry->m_data_size);

        rfs->m_spack_size = entry_start + entry_size;
    }
}

int spack_rfs_attach_data(spack_rfs_t rfs, void * data, size_t data_size) {
    struct spack_head * head;
    
    if (rfs->m_data) {
        CPE_ERROR(rfs->m_em, "spack_rfs_attach_data: already have data!");
        return -1;
    }

    if (data_size < sizeof(struct spack_head)) {
        CPE_ERROR(rfs->m_em, "spack_rfs_attach_data: buf-size=%d too small!", (int)data_size);
        return -1;
    }

    head = data;
    
    CPE_COPY_NTOH32(&rfs->m_entry_count, &head->m_entry_count);
    if (rfs->m_entry_count * sizeof(struct spack_entry) + sizeof(struct spack_head) > data_size) {
        CPE_ERROR(rfs->m_em, "spack_rfs_attach_data: entry count %d, buf-size %d too small", rfs->m_entry_count, (int)data_size);
        rfs->m_entry_count = 0;
        return -1;
    }
    
    rfs->m_data = data;
    rfs->m_data_capacity = (uint32_t)data_size;
    rfs->m_data_size = (uint32_t)data_size;
    rfs->m_data_own = 0;

    spack_rfs_read_total_size(rfs, data, data_size);
    
    if (spack_rfs_build_entries(rfs) != 0) {
        spack_rfs_clear(rfs);
        return -1;
    }
    
    return 0;
}

static int spack_rfs_append_keep_data(spack_rfs_t rfs, void * data, uint32_t data_size, uint32_t capacity) {
    assert(rfs->m_data_size + data_size <= capacity);
    
    if (capacity > rfs->m_data_capacity) {
        void * new_data = mem_alloc(rfs->m_alloc, capacity);
        if (new_data == NULL) {
            CPE_ERROR(rfs->m_em, "spack_rfs_append_data: alloc head data fail, capacity=%d!", capacity);
            return -1;
        }

        if (rfs->m_data) {
            memcpy(new_data, rfs->m_data, rfs->m_data_size);
            assert(rfs->m_data_own);
            mem_free(rfs->m_alloc, rfs->m_data);
        }
        else {
            rfs->m_data_own = 1;
        }
            
        rfs->m_data = new_data;
        rfs->m_data_capacity = capacity;
    }

    memcpy(((char *)rfs->m_data) + rfs->m_data_size, data, data_size);
    rfs->m_data_size += data_size;
    
    return 0;
}

int spack_rfs_append_data(spack_rfs_t rfs, void * data, size_t data_size) {
    uint32_t total_data_size;
    uint32_t with_entry_data_size;
    
    if (rfs->m_read_error) {
        CPE_ERROR(rfs->m_em, "spack_rfs_append_data: already read error!");
        return -1;
    }

    if (rfs->m_data && !rfs->m_data_own) {
        CPE_ERROR(rfs->m_em, "spack_rfs_append_data: already have attach data!");
        goto APPEND_ERROR;
    }

    total_data_size = rfs->m_data_size + (uint32_t)data_size;

    /*当前数据不足包头，保存起来退出 */
    if (total_data_size < sizeof(struct spack_head)) {
        if (spack_rfs_append_keep_data(rfs, data, (uint32_t)data_size, sizeof(struct spack_head)) != 0) goto APPEND_ERROR;
        return 0;
    }

    /*第一次head收取完成, 读取文件数量 */
    if (rfs->m_data_size < sizeof(struct spack_head)) {
        struct spack_head head_buf;
        struct spack_head * head;

        if (rfs->m_data) {
            assert(rfs->m_data_size <= sizeof(head));
            memcpy(&head_buf, rfs->m_data, rfs->m_data_size);
            memcpy(((char*)&head_buf) + rfs->m_data_size, data, sizeof(head) - rfs->m_data_size);
            head = &head_buf;
        }
        else {
            head = (struct spack_head *)data;
        }

        CPE_COPY_NTOH32(&rfs->m_entry_count, &head->m_entry_count);
    }

    /*计算包括所有entry的数据大小 */
    with_entry_data_size = sizeof(struct spack_head) + rfs->m_entry_count * sizeof(struct spack_entry);

    /*包括entry的数据不足，将数据缓存起来后续处理 */
    if (total_data_size < with_entry_data_size) {
        if (spack_rfs_append_keep_data(rfs, data, (uint32_t)data_size, with_entry_data_size) != 0) goto APPEND_ERROR;
        return 0;
    }

    /*完整文件大小没有计算，则计算出来 */
    if (rfs->m_spack_size == 0) spack_rfs_read_total_size(rfs, data, data_size);
    if (total_data_size > rfs->m_spack_size) {
        CPE_ERROR(rfs->m_em, "spack_rfs_append_data: total size %d overflow, spack-size=%d!", total_data_size, rfs->m_spack_size);
        goto APPEND_ERROR; 
    }
    
    /*确保缓存大小已经是完整的大小 */
    if (spack_rfs_append_keep_data(rfs, data, (uint32_t)data_size, rfs->m_spack_size) != 0) goto APPEND_ERROR;
    
    return 0;

APPEND_ERROR:
    rfs->m_read_error = 1;
    return -1;
}

int spack_rfs_append_complete(spack_rfs_t rfs) {
    if (rfs->m_read_error) {
        CPE_ERROR(rfs->m_em, "spack_rfs_append_complete: already read error!");
        return -1;
    }

    if (rfs->m_spack_size == 0) {
        CPE_ERROR(rfs->m_em, "spack_rfs_append_complete: no data!");
        return -1;
    }

    if (rfs->m_data_size < rfs->m_spack_size) {
        CPE_ERROR(rfs->m_em, "spack_rfs_append_complete: data not enough, spack-size=%d, but only %d!", rfs->m_spack_size, rfs->m_data_size);
        return -1;
    }

    if(rfs->m_data_size > rfs->m_spack_size) {
        CPE_ERROR(rfs->m_em, "spack_rfs_append_complete: data overflow, spack-size=%d, data-size=%d!", rfs->m_spack_size, rfs->m_data_size);
        return -1;
    }

    if (spack_rfs_build_entries(rfs) != 0) {
        spack_rfs_clear(rfs);
        return -1;
    }
    
    return 0;
}
