#ifndef CPE_CFG_CALC_H
#define CPE_CFG_CALC_H
#include "cpe/xcalc/xcalc_types.h"
#include "cfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cfg_calc_context {
    cfg_t m_cfg;
    cfg_calc_context_t m_next;
};
    
xtoken_t cfg_calc_find_value(void * input_ctx, xcomputer_t computer, const char * attr_name, error_monitor_t em);

uint8_t cfg_calc_bool_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, int8_t dft);
int8_t cfg_calc_int8_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, int8_t dft);
uint8_t cfg_calc_uint8_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, uint8_t dft);
int16_t cfg_calc_int16_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, int16_t dft);
uint16_t cfg_calc_uint16_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, uint16_t dft);
int32_t cfg_calc_int32_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, int32_t dft);
uint32_t cfg_calc_uint32_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, uint32_t dft);
int64_t cfg_calc_int64_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, int64_t dft);
uint64_t cfg_calc_uint64_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, uint64_t dft);
float cfg_calc_float_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, float dft);
double cfg_calc_double_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, double dft);
const char * cfg_calc_str_with_dft(xcomputer_t computer, mem_buffer_t buffer, const char * def, cfg_calc_context_t ctx, const char * dft);
char * cfg_calc_str_with_dft_dup(xcomputer_t computer, mem_allocrator_t alloc, const char * def, cfg_calc_context_t ctx, const char * dft);

int cfg_try_calc_bool(xcomputer_t computer, uint8_t * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em);
int cfg_try_calc_int8(xcomputer_t computer, int8_t * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em);
int cfg_try_calc_uint8(xcomputer_t computer, uint8_t * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em);
int cfg_try_calc_int16(xcomputer_t computer, int16_t * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em);
int cfg_try_calc_uint16(xcomputer_t computer, uint16_t * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em);
int cfg_try_calc_int32(xcomputer_t computer, int32_t * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em);
int cfg_try_calc_uint32(xcomputer_t computer, uint32_t * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em);
int cfg_try_calc_int64(xcomputer_t computer, int64_t * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em);
int cfg_try_calc_uint64(xcomputer_t computer, uint64_t * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em);
int cfg_try_calc_float(xcomputer_t computer, float * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em);
int cfg_try_calc_double(xcomputer_t computer, double * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em);
const char * cfg_try_calc_str(xcomputer_t computer, mem_buffer_t buffer, const char * def, cfg_calc_context_t ctx, error_monitor_t em);
char * cfg_try_calc_str_dup(xcomputer_t computer, mem_allocrator_t alloc, const char * def, cfg_calc_context_t ctx, error_monitor_t em);
    
#ifdef __cplusplus
}
#endif

#endif


