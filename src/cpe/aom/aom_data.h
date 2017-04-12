#ifndef CPE_AOM_INTERNAL_DATA_H
#define CPE_AOM_INTERNAL_DATA_H
#include "aom_internal_types.h"

#define OM_GRP_OBJ_CONTROL_MAGIC (38438u)

struct aom_obj_control_data {
    uint16_t m_magic;
    uint16_t m_head_version;
    char m_meta_name[64];
    uint32_t m_metalib_start;
    uint32_t m_metalib_size;
    uint32_t m_data_start;
    uint32_t m_data_size;
};

#endif
