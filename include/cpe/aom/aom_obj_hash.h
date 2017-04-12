#ifndef CPE_AOM_HASH_H
#define CPE_AOM_HASH_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/dr/dr_types.h"
#include "aom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t (*aom_obj_hash_fun_t)(const void * input, LPDRMETA meta);
typedef int (*aom_obj_cmp_fun_t)(const void * l, const void * r, LPDRMETA meta);

aom_obj_hash_table_t
aom_obj_hash_table_create(
    mem_allocrator_t alloc, error_monitor_t em,
    aom_obj_mgr_t mgr, 
    aom_obj_hash_fun_t hash_fun,
    aom_obj_cmp_fun_t cmp_fun,
    void * buff, size_t buff_capacity);

void aom_obj_hash_table_free(aom_obj_hash_table_t hs_table);

size_t aom_obj_hash_table_buff_capacity(aom_obj_hash_table_t hs_table);

void * aom_obj_hash_table_find(aom_obj_hash_table_t hs_table, void const * key);
void * aom_obj_hash_table_find_next(aom_obj_hash_table_t hs_table, void const * obj);

int aom_obj_hash_table_insert_unique(aom_obj_hash_table_t hs_table, void const * data, ptr_int_t * record_id);
int aom_obj_hash_table_insert(aom_obj_hash_table_t hs_table, void const * data, ptr_int_t * record_id);

int aom_obj_hash_table_insert_or_update(aom_obj_hash_table_t hs_table, void const * data, ptr_int_t * record_id);

int aom_obj_hash_table_remove_by_ins(aom_obj_hash_table_t hs_table, void * data);
int aom_obj_hash_table_remove_by_key(aom_obj_hash_table_t hs_table, void const * key);
int aom_obj_hash_table_remove_all_by_key(aom_obj_hash_table_t hs_table, void const * key);

size_t aom_obj_hash_table_buf_calc_capacity(aom_obj_mgr_t mgr, float bucket_ratio);
int aom_obj_hash_table_buf_init(
    aom_obj_mgr_t mgr, float bucket_ratio, 
    aom_obj_hash_fun_t hash_fun,
    void * buf, size_t buf_capacity, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
