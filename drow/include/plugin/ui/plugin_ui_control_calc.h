#ifndef DROW_PLUGIN_UI_CONTROL_CALC_H
#define DROW_PLUGIN_UI_CONTROL_CALC_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t plugin_ui_control_calc_bool_with_dft(const char * def, plugin_ui_control_t control, dr_data_source_t data_source, int8_t dft);
int8_t plugin_ui_control_calc_int8_with_dft(const char * def, plugin_ui_control_t control, dr_data_source_t data_source, int8_t dft);
uint8_t plugin_ui_control_calc_uint8_with_dft(const char * def, plugin_ui_control_t control, dr_data_source_t data_source, uint8_t dft);
int16_t plugin_ui_control_calc_int16_with_dft(const char * def, plugin_ui_control_t control, dr_data_source_t data_source, int16_t dft);
uint16_t plugin_ui_control_calc_uint16_with_dft(const char * def, plugin_ui_control_t control, dr_data_source_t data_source, uint16_t dft);
int32_t plugin_ui_control_calc_int32_with_dft(const char * def, plugin_ui_control_t control, dr_data_source_t data_source, int32_t dft);
uint32_t plugin_ui_control_calc_uint32_with_dft(const char * def, plugin_ui_control_t control, dr_data_source_t data_source, uint32_t dft);
int64_t plugin_ui_control_calc_int64_with_dft(const char * def, plugin_ui_control_t control, dr_data_source_t data_source, int64_t dft);
uint64_t plugin_ui_control_calc_uint64_with_dft(const char * def, plugin_ui_control_t control, dr_data_source_t data_source, uint64_t dft);
float plugin_ui_control_calc_float_with_dft(const char * def, plugin_ui_control_t control, dr_data_source_t data_source, float dft);
double plugin_ui_control_calc_double_with_dft(const char * def, plugin_ui_control_t control, dr_data_source_t data_source, double dft);
const char * plugin_ui_control_calc_str_with_dft(
    mem_buffer_t buffer, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, const char * dft);

int plugin_ui_control_try_calc_bool(uint8_t * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_try_calc_int8(int8_t * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_try_calc_uint8(uint8_t * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_try_calc_int16(int16_t * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_try_calc_uint16(uint16_t * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_try_calc_int32(int32_t * result, const char * def, plugin_ui_control_t control, dr_data_source_t daityta_source, error_monitor_t em);
int plugin_ui_control_try_calc_uint32(uint32_t * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_try_calc_int64(int64_t * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_try_calc_uint64(uint64_t * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_try_calc_float(float * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_try_calc_double(double * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
const char * plugin_ui_control_try_calc_str(
    mem_buffer_t buffer, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);

/*check系列会检测传入的字符串是否是公式并分别处理 */
int plugin_ui_control_check_calc_bool(uint8_t * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_check_calc_int8(int8_t * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_check_calc_uint8(uint8_t * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_check_calc_int16(int16_t * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_check_calc_uint16(uint16_t * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_check_calc_int32(int32_t * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_check_calc_uint32(uint32_t * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_check_calc_int64(int64_t * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_check_calc_uint64(uint64_t * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_check_calc_float(float * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
int plugin_ui_control_check_calc_double(double * result, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);
const char * plugin_ui_control_check_calc_str(
    mem_buffer_t buffer, const char * def, plugin_ui_control_t control, dr_data_source_t data_source, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif

