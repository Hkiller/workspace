#ifndef CPE_SPACK_REPO_I_H
#define CPE_SPACK_REPO_I_H
#include "cpe/utils/hash.h"
#include "cpe/spack/spack_repo.h"

struct spack_repo {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    struct cpe_hash_table m_files;
    void const * m_buf;
    size_t m_buf_size;
};

#endif
