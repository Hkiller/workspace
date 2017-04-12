#ifndef USF_LOGIC_USE_DATA_DYN_H
#define USF_LOGIC_USE_DATA_DYN_H
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "usf/logic/logic_types.h"
#include "logic_use_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int logic_data_calc_capacity(LPDRMETA meta, size_t record_capacity, error_monitor_t em);
int logic_data_record_is_dyn(logic_data_t data);

LPDRMETA logic_data_record_meta(logic_data_t data);
int logic_data_record_count(logic_data_t data);
int logic_data_record_set_count(logic_data_t data, size_t record_count);
void * logic_data_record_append(logic_data_t data);
void * logic_data_record_append_auto_inc(logic_data_t * data);
int logic_data_record_remove_by_pos(logic_data_t data, size_t pos);
int logic_data_record_remove_by_ins(logic_data_t data, void * obj);

size_t logic_data_record_capacity(logic_data_t data);
logic_data_t logic_data_record_reserve(logic_data_t data, size_t record_capacity);

void * logic_data_record_at(logic_data_t data, int pos);
size_t logic_data_record_size(logic_data_t data);

void logic_data_record_sort(logic_data_t data, int(*cmp)(const void *, const void *));
void * logic_data_record_find(logic_data_t data, void const * key, int(*cmp)(const void *, const void *));

#ifdef __cplusplus
}
#endif

#endif
