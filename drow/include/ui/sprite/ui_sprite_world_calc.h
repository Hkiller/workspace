#ifndef UI_SPRITE_WORLD_CALC_H
#define UI_SPRITE_WORLD_CALC_H
#include "cpe/utils/buffer.h"
#include "ui_sprite_types.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t ui_sprite_world_calc_bool_with_dft(const char * def, ui_sprite_world_t world, dr_data_source_t data_source, int8_t dft);
int8_t ui_sprite_world_calc_int8_with_dft(const char * def, ui_sprite_world_t world, dr_data_source_t data_source, int8_t dft);
uint8_t ui_sprite_world_calc_uint8_with_dft(const char * def, ui_sprite_world_t world, dr_data_source_t data_source, uint8_t dft);
int16_t ui_sprite_world_calc_int16_with_dft(const char * def, ui_sprite_world_t world, dr_data_source_t data_source, int16_t dft);
uint16_t ui_sprite_world_calc_uint16_with_dft(const char * def, ui_sprite_world_t world, dr_data_source_t data_source, uint16_t dft);
int32_t ui_sprite_world_calc_int32_with_dft(const char * def, ui_sprite_world_t world, dr_data_source_t data_source, int32_t dft);
uint32_t ui_sprite_world_calc_uint32_with_dft(const char * def, ui_sprite_world_t world, dr_data_source_t data_source, uint32_t dft);
int64_t ui_sprite_world_calc_int64_with_dft(const char * def, ui_sprite_world_t world, dr_data_source_t data_source, int64_t dft);
uint64_t ui_sprite_world_calc_uint64_with_dft(const char * def, ui_sprite_world_t world, dr_data_source_t data_source, uint64_t dft);
float ui_sprite_world_calc_float_with_dft(const char * def, ui_sprite_world_t world, dr_data_source_t data_source, float dft);
double ui_sprite_world_calc_double_with_dft(const char * def, ui_sprite_world_t world, dr_data_source_t data_source, double dft);
const char * ui_sprite_world_calc_str_with_dft(
    mem_buffer_t buffer, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, const char * dft);
char * ui_sprite_world_calc_str_with_dft_dup(
    mem_allocrator_t alloc, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, const char * dft);

int ui_sprite_world_try_calc_bool(uint8_t * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_try_calc_int8(int8_t * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_try_calc_uint8(uint8_t * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_try_calc_int16(int16_t * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_try_calc_uint16(uint16_t * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_try_calc_int32(int32_t * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_try_calc_uint32(uint32_t * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_try_calc_int64(int64_t * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_try_calc_uint64(uint64_t * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_try_calc_float(float * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_try_calc_double(double * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
const char * ui_sprite_world_try_calc_str(
    mem_buffer_t buffer, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
char * ui_sprite_world_try_calc_str_dup(
    mem_allocrator_t alloc, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);

/*check系列会检测传入的字符串是否是公式并分别处理 */
int ui_sprite_world_check_calc_bool(uint8_t * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_check_calc_int8(int8_t * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_check_calc_uint8(uint8_t * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_check_calc_int16(int16_t * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_check_calc_uint16(uint16_t * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_check_calc_int32(int32_t * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_check_calc_uint32(uint32_t * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_check_calc_int64(int64_t * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_check_calc_uint64(uint64_t * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_check_calc_float(float * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_world_check_calc_double(double * result, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
const char * ui_sprite_world_check_calc_str(
    mem_buffer_t buffer, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
char * ui_sprite_world_check_calc_str_dup(
    mem_allocrator_t alloc, const char * def, ui_sprite_world_t world, dr_data_source_t data_source, error_monitor_t em);
    
#ifdef __cplusplus
}
#endif

#endif
