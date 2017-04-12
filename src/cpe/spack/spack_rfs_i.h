#ifndef CPE_SPACK_RFS_I_H
#define CPE_SPACK_RFS_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/error.h"
#include "cpe/utils/memory.h"
#include "cpe/spack/spack_rfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spack_rfs_backend * spack_rfs_backend_t;
typedef struct spack_rfs_entry * spack_rfs_entry_t;
typedef struct spack_rfs_file * spack_rfs_file_t;
typedef struct spack_rfs_dir * spack_rfs_dir_t;

typedef TAILQ_HEAD(spack_rfs_list, spack_rfs) spack_rfs_list_t;
typedef TAILQ_HEAD(spack_rfs_entry_list, spack_rfs_entry) spack_rfs_entry_list_t;
    
struct spack_rfs {
    vfs_mgr_t m_vfs;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    spack_rfs_backend_t m_backend;
    TAILQ_ENTRY(spack_rfs) m_next;
    vfs_mount_point_t m_mount_point;
    spack_rfs_entry_t m_root;

    /*文件中读取的信息 */
    uint32_t m_entry_count;    /*文件中包含的entry数量 */
    uint32_t m_spack_size;     /*需要的数据大小 */

    /*错误状态 */
    uint8_t m_read_error;

    /*缓存管理 */
    uint8_t m_data_own;
    uint32_t m_data_capacity;
    uint32_t m_data_size;
    void * m_data;
};

vfs_backend_t spack_rfs_create_backend(vfs_mgr_t vfs, mem_allocrator_t alloc, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
