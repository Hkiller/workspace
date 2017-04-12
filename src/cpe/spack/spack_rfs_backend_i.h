#ifndef CPE_SPACK_RFS_BACKEND_I_H
#define CPE_SPACK_RFS_BACKEND_I_H
#include "spack_rfs_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct spack_rfs_backend {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    spack_rfs_list_t m_rfss;
};

struct spack_rfs_file {
    spack_rfs_entry_t m_entry;
    ssize_t m_pos;
};

struct spack_rfs_dir {
    spack_rfs_entry_t m_entry;
};

#ifdef __cplusplus
}
#endif

#endif
