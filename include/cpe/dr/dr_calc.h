#ifndef CPE_DR_CALC_H
#define CPE_DR_CALC_H
#include "cpe/xcalc/xcalc_types.h"
#include "dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

xtoken_t dr_create_token_from_ctype(xcomputer_t computer, uint8_t type, void * data);
xtoken_t dr_create_token_from_entry(xcomputer_t computer, dr_data_entry_t entry);
xtoken_t dr_create_token_from_value(xcomputer_t computer, dr_value_t value);

uint8_t dr_calc_bool_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, int8_t dft);
int8_t dr_calc_int8_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, int8_t dft);
uint8_t dr_calc_uint8_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, uint8_t dft);
int16_t dr_calc_int16_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, int16_t dft);
uint16_t dr_calc_uint16_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, uint16_t dft);
int32_t dr_calc_int32_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, int32_t dft);
uint32_t dr_calc_uint32_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, uint32_t dft);
int64_t dr_calc_int64_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, int64_t dft);
uint64_t dr_calc_uint64_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, uint64_t dft);
float dr_calc_float_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, float dft);
double dr_calc_double_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, double dft);
const char * dr_calc_str_with_dft(mem_buffer_t buffer, xcomputer_t computer, const char * def, dr_data_source_t data_source, const char * dft);

int dr_try_calc_bool(uint8_t * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em);
int dr_try_calc_int8(int8_t * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em);
int dr_try_calc_uint8(uint8_t * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em);
int dr_try_calc_int16(int16_t * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em);
int dr_try_calc_uint16(uint16_t * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em);
int dr_try_calc_int32(int32_t * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em);
int dr_try_calc_uint32(uint32_t * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em);
int dr_try_calc_int64(int64_t * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em);
int dr_try_calc_uint64(uint64_t * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em);
int dr_try_calc_float(float * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em);
int dr_try_calc_double(double * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em);
const char * dr_try_calc_str(mem_buffer_t buffer, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em);
    
#ifdef __cplusplus
}
#endif

#endif
