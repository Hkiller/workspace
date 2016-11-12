#ifndef CPE_AOM_MANAGE_H
#define CPE_AOM_MANAGE_H
#include "cpe/utils/stream.h"
#include "cpe/utils/memory.h"
#include "cpe/dr/dr_types.h"
#include "aom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

aom_obj_mgr_t
aom_obj_mgr_create(
    mem_allocrator_t alloc,
    void * data,
    size_t data_capacity,
    error_monitor_t em);

void aom_obj_mgr_free(aom_obj_mgr_t mgr);

void * aom_obj_mgr_data(aom_obj_mgr_t mgr);
size_t aom_obj_mgr_data_capacity(aom_obj_mgr_t mgr);

uint32_t aom_obj_mgr_free_obj_count(aom_obj_mgr_t mgr);
uint32_t aom_obj_mgr_allocked_obj_count(aom_obj_mgr_t mgr);

LPDRMETA aom_obj_mgr_meta(aom_obj_mgr_t mgr);

void * aom_obj_alloc(aom_obj_mgr_t mgr);
void aom_obj_free(aom_obj_mgr_t mgr, void * obj);
void aom_objs(aom_obj_mgr_t mgr, aom_obj_it_t it);

void aom_obj_free_by_idx(aom_obj_mgr_t mgr, ptr_int_t idx);

void * aom_obj_get(aom_obj_mgr_t mgr, ptr_int_t idx);
ptr_int_t aom_obj_index(aom_obj_mgr_t mgr, void const * obj);

int aom_obj_mgr_buf_calc_capacity(
    size_t * result, 
    LPDRMETA meta, uint32_t record_count,
    error_monitor_t em);

int aom_obj_mgr_buf_init(
    LPDRMETA meta,
    void * data, size_t data_capacity,
    error_monitor_t em);

void aom_obj_mgr_info(aom_obj_mgr_t mgr, write_stream_t stream, int ident);

#define aom_obj_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
