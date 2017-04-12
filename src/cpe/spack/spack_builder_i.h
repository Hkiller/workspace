#ifndef CPE_SPACK_BUILDER_I_H
#define CPE_SPACK_BUILDER_I_H
#include "cpe/utils/hash.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/spack/spack_builder.h"

typedef struct spack_building_file * spack_building_file_t;

struct spack_builder {
    vfs_mgr_t m_vfs;
    const char * m_root;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    struct cpe_hash_table m_files;
    struct mem_buffer m_string_buff;
};

#endif
