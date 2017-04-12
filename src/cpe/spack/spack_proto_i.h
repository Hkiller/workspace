#ifndef CPE_SPACK_PROTO_I_H
#define CPE_SPACK_PROTO_I_H
#include "spack_repo_i.h"

#pragma pack(push,1)

struct spack_head {
    uint32_t m_magic;
    uint32_t m_entry_count;
};

struct spack_entry {
    uint32_t m_name;
    uint32_t m_data_start;
    uint32_t m_data_size;
};

#pragma pack(pop)

#endif
