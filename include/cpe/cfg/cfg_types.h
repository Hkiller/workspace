#ifndef CPE_CFG_TYPES_H
#define CPE_CFG_TYPES_H
#include "cpe/pal/pal_types.h"
#include "cpe/dr/dr_ctypes_info.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cfg * cfg_t;

typedef enum cfg_policy {
    cfg_merge_use_new,
    cfg_merge_use_exist,
    cfg_replace
} cfg_policy_t;

struct cfg_struct_it {
    cfg_t m_curent;
};

struct cfg_seq_it {
    void * m_block;
    int32_t m_pos_in_block;
    int32_t m_left_count;
};

union cfg_it_data {
    struct cfg_struct_it m_struct_it;
    struct cfg_seq_it m_seq_it;
};

typedef struct cfg_it {
    cfg_t (*next)(struct cfg_it * it);
    union cfg_it_data m_data;
} cfg_it_t;

typedef struct cfg_calc_context * cfg_calc_context_t;

#define CPE_CFG_NAME_MAX_LEN (128)
#define CPE_CFG_MAX_LEVEL (128)

#define CPE_CFG_TYPE_SEQUENCE    CPE_DR_TYPE_UNION
#define CPE_CFG_TYPE_STRUCT      CPE_DR_TYPE_STRUCT
#define CPE_CFG_TYPE_INT8        CPE_DR_TYPE_INT8
#define CPE_CFG_TYPE_UINT8       CPE_DR_TYPE_UINT8
#define CPE_CFG_TYPE_INT16       CPE_DR_TYPE_INT16
#define CPE_CFG_TYPE_UINT16      CPE_DR_TYPE_UINT16
#define CPE_CFG_TYPE_INT32       CPE_DR_TYPE_INT32
#define CPE_CFG_TYPE_UINT32      CPE_DR_TYPE_UINT32
#define CPE_CFG_TYPE_INT64       CPE_DR_TYPE_INT64
#define CPE_CFG_TYPE_UINT64      CPE_DR_TYPE_UINT64
#define CPE_CFG_TYPE_STRING      CPE_DR_TYPE_STRING
#define CPE_CFG_TYPE_FLOAT       CPE_DR_TYPE_FLOAT
#define CPE_CFG_TYPE_DOUBLE      CPE_DR_TYPE_DOUBLE

enum cfg_bin_write_error {
    cfg_bin_write_error_not_enouth_output_buf = -2
};

#ifdef __cplusplus
}
#endif

#endif


