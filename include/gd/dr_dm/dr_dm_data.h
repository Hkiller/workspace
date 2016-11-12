#ifndef GD_DR_DM_DATA_H
#define GD_DR_DM_DATA_H
#include "dr_dm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dr_dm_data_t
dr_dm_data_create(
    dr_dm_manage_t mgr,
    const void * data, size_t data_size,
    const char ** duplicate_index);

void dr_dm_data_free(dr_dm_data_t dr_dm_data);

dr_dm_data_id_t dr_dm_data_id(dr_dm_data_t dr_dm_data);
void * dr_dm_data_data(dr_dm_data_t dr_dm_data);
size_t dr_dm_data_data_capacity(dr_dm_data_t dr_dm_data);
LPDRMETA dr_dm_data_meta(dr_dm_data_t dr_dm_data);

dr_dm_data_t dr_dm_data_find_by_id(dr_dm_manage_t mgr, dr_dm_data_id_t id);

dr_dm_data_t dr_dm_data_find_by_index_int8(dr_dm_manage_t mgr, const char * idx_name, int8_t input);
dr_dm_data_t dr_dm_data_find_by_index_uint8(dr_dm_manage_t mgr, const char * idx_name, uint8_t input);
dr_dm_data_t dr_dm_data_find_by_index_int16(dr_dm_manage_t mgr, const char * idx_name, int16_t input);
dr_dm_data_t dr_dm_data_find_by_index_uint16(dr_dm_manage_t mgr, const char * idx_name, uint16_t input);
dr_dm_data_t dr_dm_data_find_by_index_int32(dr_dm_manage_t mgr, const char * idx_name, int32_t input);
dr_dm_data_t dr_dm_data_find_by_index_uint32(dr_dm_manage_t mgr, const char * idx_name, uint32_t input);
dr_dm_data_t dr_dm_data_find_by_index_int64(dr_dm_manage_t mgr, const char * idx_name, int64_t input);
dr_dm_data_t dr_dm_data_find_by_index_uint64(dr_dm_manage_t mgr, const char * idx_name, uint64_t input);
dr_dm_data_t dr_dm_data_find_by_index_float(dr_dm_manage_t mgr, const char * idx_name, float input);
dr_dm_data_t dr_dm_data_find_by_index_double(dr_dm_manage_t mgr, const char * idx_name, double input);
dr_dm_data_t dr_dm_data_find_by_index_string(dr_dm_manage_t mgr, const char * idx_name, const char * input);
dr_dm_data_t dr_dm_data_find_by_index_ctype(dr_dm_manage_t mgr, const char * idx_name, const void * input, int input_type);

void dr_dm_data_it_init(struct dr_dm_data_it * it, dr_dm_manage_t mgr);
#define dr_dm_data_it_next(__it) ((__it)->next(__it));

#ifdef __cplusplus
}
#endif

#endif
