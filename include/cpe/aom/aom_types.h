#ifndef CPE_AOM_TYPES_H
#define CPE_AOM_TYPES_H
#include "cpe/pal/pal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct aom_obj_mgr * aom_obj_mgr_t;
typedef struct aom_obj_hash_table * aom_obj_hash_table_t;

typedef struct aom_obj_it {
    void * (*next)(struct aom_obj_it * it);
    char m_data[64];
} * aom_obj_it_t;

enum aom_obj_hash_table_error {
    aom_obj_hash_table_error_duplicate = 1
    , aom_obj_hash_table_error_not_exist = 2
    , aom_obj_hash_table_error_obj_not_managed = 3
    , aom_obj_hash_table_error_no_memory = 4
};

#ifdef __cplusplus
}
#endif

#endif
