#ifndef CPE_DR_DATA_VALUE_H
#define CPE_DR_DATA_VALUE_H
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int8_t dr_value_read_with_dft_int8(dr_value_t value, int8_t dft);
uint8_t dr_value_read_with_dft_uint8(dr_value_t value, uint8_t dft);
int16_t dr_value_read_with_dft_int16(dr_value_t value, int16_t dft);
uint16_t dr_value_read_with_dft_uint16(dr_value_t value, uint16_t dft);
int32_t dr_value_read_with_dft_int32(dr_value_t value, int32_t dft);
uint32_t dr_value_read_with_dft_uint32(dr_value_t value, uint32_t dft);
int64_t dr_value_read_with_dft_int64(dr_value_t value, int64_t dft);
uint64_t dr_value_read_with_dft_uint64(dr_value_t value, uint64_t dft);
float dr_value_read_with_dft_float(dr_value_t value, float dft);
double dr_value_read_with_dft_double(dr_value_t value, double dft);
const char * dr_value_read_with_dft_string(dr_value_t value, const char * dft);
const char * dr_value_to_string(mem_buffer_t buf, dr_value_t value, const char * dft);
    
int dr_value_try_read_int8(int8_t * result, dr_value_t value, error_monitor_t em);
int dr_value_try_read_uint8(uint8_t * result, dr_value_t value, error_monitor_t em);
int dr_value_try_read_int16(int16_t * result, dr_value_t value, error_monitor_t em);
int dr_value_try_read_uint16(uint16_t * result, dr_value_t value, error_monitor_t em);
int dr_value_try_read_int32(int32_t * result, dr_value_t value, error_monitor_t em);
int dr_value_try_read_uint32(uint32_t * result, dr_value_t value, error_monitor_t em);
int dr_value_try_read_int64(int64_t * result, dr_value_t value, error_monitor_t em);
int dr_value_try_read_uint64(uint64_t * result, dr_value_t value, error_monitor_t em);
int dr_value_try_read_float(float * result, dr_value_t value, error_monitor_t em);
int dr_value_try_read_double(double * result, dr_value_t value, error_monitor_t em);
const char * dr_value_try_read_string(dr_value_t value, error_monitor_t em);

dr_value_t dr_value_find_by_path(dr_data_t data, const char * path, dr_value_t value_buf);
int dr_value_set_from_value(dr_value_t to, dr_value_t from, error_monitor_t em);
    
#ifdef __cplusplus
}
#endif

#endif
