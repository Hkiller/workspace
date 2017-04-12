#ifndef CPE_DR_READ_H
#define CPE_DR_READ_H
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "cpe/utils/stream.h"
#include "dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int dr_entry_try_read_int8(int8_t * result, const void * input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_try_read_uint8(uint8_t * result, const void * input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_try_read_int16(int16_t * result, const void * input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_try_read_uint16(uint16_t * result, const void * input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_try_read_int32(int32_t * result, const void * input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_try_read_uint32(uint32_t * result, const void * input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_try_read_int64(int64_t * result, const void * input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_try_read_uint64(uint64_t * result, const void * input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_try_read_float(float * result, const void * input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_try_read_double(double * result, const void * input, LPDRMETAENTRY entry, error_monitor_t em);

int8_t dr_entry_read_int8(const void * input, LPDRMETAENTRY entry);
uint8_t dr_entry_read_uint8(const void * input, LPDRMETAENTRY entry);
int16_t dr_entry_read_int16(const void * input, LPDRMETAENTRY entry);
uint16_t dr_entry_read_uint16(const void * input, LPDRMETAENTRY entry);
int32_t dr_entry_read_int32(const void * input, LPDRMETAENTRY entry);
uint32_t dr_entry_read_uint32(const void * input, LPDRMETAENTRY entry);
int64_t dr_entry_read_int64(const void * input, LPDRMETAENTRY entry);
uint64_t dr_entry_read_uint64(const void * input, LPDRMETAENTRY entry);
float dr_entry_read_float(const void * input, LPDRMETAENTRY entry);
double dr_entry_read_double(const void * input, LPDRMETAENTRY entry);
const char * dr_entry_read_string(const void * input, LPDRMETAENTRY entry);

int8_t dr_entry_read_with_dft_int8(const void * input, LPDRMETAENTRY entry, int8_t dft);
uint8_t dr_entry_read_with_dft_uint8(const void * input, LPDRMETAENTRY entry, uint8_t dft);
int16_t dr_entry_read_with_dft_int16(const void * input, LPDRMETAENTRY entry, int16_t dft);
uint16_t dr_entry_read_with_dft_uint16(const void * input, LPDRMETAENTRY entry, uint16_t dft);
int32_t dr_entry_read_with_dft_int32(const void * input, LPDRMETAENTRY entry, int32_t dft);
uint32_t dr_entry_read_with_dft_uint32(const void * input, LPDRMETAENTRY entry, uint32_t dft);
int64_t dr_entry_read_with_dft_int64(const void * input, LPDRMETAENTRY entry, int64_t dft);
uint64_t dr_entry_read_with_dft_uint64(const void * input, LPDRMETAENTRY entry, uint64_t dft);
float dr_entry_read_with_dft_float(const void * input, LPDRMETAENTRY entry, float dft);
double dr_entry_read_with_dft_double(const void * input, LPDRMETAENTRY entry, double dft);
const char * dr_entry_read_with_dft_string(const void * input, LPDRMETAENTRY entry, const char * dft);

int dr_entry_print_to_stream(write_stream_t output, const void * input, LPDRMETAENTRY entry, error_monitor_t em);
const char * dr_entry_to_string(mem_buffer_t buf, const void * input, LPDRMETAENTRY entry);

int dr_entry_set_from_int8(void * output, int8_t input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_set_from_uint8(void * output, uint8_t input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_set_from_int16(void * output, int16_t input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_set_from_uint16(void * output, uint16_t input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_set_from_int32(void * output, int32_t input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_set_from_uint32(void * output, uint32_t input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_set_from_int64(void * output, int64_t input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_set_from_uint64(void * output, uint64_t input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_set_from_float(void * output, float input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_set_from_double(void * output, double input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_set_from_string(void * output, const char * input, LPDRMETAENTRY entry, error_monitor_t em);
int dr_entry_set_from_ctype(void * output, const void * input, int input_type, LPDRMETAENTRY entry, error_monitor_t em);

int dr_meta_set_from_int8(void * output, int8_t input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_set_from_uint8(void * output, uint8_t input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_set_from_int16(void * output, int16_t input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_set_from_uint16(void * output, uint16_t input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_set_from_int32(void * output, int32_t input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_set_from_uint32(void * output, uint32_t input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_set_from_int64(void * output, int64_t input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_set_from_uint64(void * output, uint64_t input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_set_from_float(void * output, float input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_set_from_double(void * output, double input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_set_from_string(void * output, const char * input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_set_from_ctype(void * output, const void * input, int input_type, LPDRMETA meta, const char * entry, error_monitor_t em);

int dr_meta_try_read_int8(int8_t * result, const void * input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_try_read_uint8(uint8_t * result, const void * input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_try_read_int16(int16_t * result, const void * input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_try_read_uint16(uint16_t * result, const void * input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_try_read_int32(int32_t * result, const void * input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_try_read_uint32(uint32_t * result, const void * input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_try_read_int64(int64_t * result, const void * input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_try_read_uint64(uint64_t * result, const void * input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_try_read_float(float * result, const void * input, LPDRMETA meta, const char * entry, error_monitor_t em);
int dr_meta_try_read_double(double * result, const void * input, LPDRMETA meta, const char * entry, error_monitor_t em);

int8_t dr_meta_read_int8(const void * input, LPDRMETA meta, const char * entry);
uint8_t dr_meta_read_uint8(const void * input, LPDRMETA meta, const char * entry);
int16_t dr_meta_read_int16(const void * input, LPDRMETA meta, const char * entry);
uint16_t dr_meta_read_uint16(const void * input, LPDRMETA meta, const char * entry);
int32_t dr_meta_read_int32(const void * input, LPDRMETA meta, const char * entry);
uint32_t dr_meta_read_uint32(const void * input, LPDRMETA meta, const char * entry);
int64_t dr_meta_read_int64(const void * input, LPDRMETA meta, const char * entry);
uint64_t dr_meta_read_uint64(const void * input, LPDRMETA meta, const char * entry);
float dr_meta_read_float(const void * input, LPDRMETA meta, const char * entry);
double dr_meta_read_double(const void * input, LPDRMETA meta, const char * entry);
const char * dr_meta_read_string(const void * input, LPDRMETA meta, const char * entry);

int8_t dr_meta_read_with_dft_int8(const void * input, LPDRMETA meta, const char * entry, int8_t dft);
uint8_t dr_meta_read_with_dft_uint8(const void * input, LPDRMETA meta, const char * entry, uint8_t dft);
int16_t dr_meta_read_with_dft_int16(const void * input, LPDRMETA meta, const char * entry, int16_t dft);
uint16_t dr_meta_read_with_dft_uint16(const void * input, LPDRMETA meta, const char * entry, uint16_t dft);
int32_t dr_meta_read_with_dft_int32(const void * input, LPDRMETA meta, const char * entry, int32_t dft);
uint32_t dr_meta_read_with_dft_uint32(const void * input, LPDRMETA meta, const char * entry, uint32_t dft);
int64_t dr_meta_read_with_dft_int64(const void * input, LPDRMETA meta, const char * entry, int64_t dft);
uint64_t dr_meta_read_with_dft_uint64(const void * input, LPDRMETA meta, const char * entry, uint64_t dft);
float dr_meta_read_with_dft_float(const void * input, LPDRMETA meta, const char * entry, float dft);
double dr_meta_read_with_dft_double(const void * input, LPDRMETA meta, const char * entry, double dft);
const char * dr_meta_read_with_dft_string(const void * input, LPDRMETA meta, const char * entry, const char * dft);

size_t dr_meta_calc_data_len(LPDRMETA meta, void const * data, size_t capacity);

#define DR_SET_DEFAULTS_POLICY_NO_DEFAULT_IGNORE 1
int dr_entry_set_defaults(void * inout, size_t capacity, LPDRMETAENTRY entry, int policy);
void dr_meta_set_defaults(void * inout, size_t capacity, LPDRMETA meta, int policy);

int dr_meta_copy_same_entry(
    void * desData, size_t desCapacity, LPDRMETA desMeta,
    void const * srcData, size_t srcCapacity, LPDRMETA srcMeta,
    int policy, error_monitor_t em);

int dr_meta_copy_same_entry_part(
    void * desData, size_t desCapacity, LPDRMETA desMeta,
    void const * srcData, size_t srcCapacity, LPDRMETA srcMeta,
    const char * columns,
    int policy, error_monitor_t em);

int dr_entry_copy_same_entry(
    void * desData, size_t desCapacity, LPDRMETAENTRY desEntry,
    void const * srcData, size_t srcCapacity, LPDRMETAENTRY srcEntry,
    int policy, error_monitor_t em);

uint32_t dr_entry_hash(const void * input, LPDRMETAENTRY entry);
int dr_entry_cmp(const void * l, const void * r, LPDRMETAENTRY entry);

uint32_t dr_meta_key_hash(const void * input, LPDRMETA meta);
int dr_meta_key_cmp(const void * l, const void * r, LPDRMETA meta);

int dr_meta_data_cmp(const void * l, const void * r, LPDRMETA meta);

#ifdef __cplusplus
}
#endif

#endif
