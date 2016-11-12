#ifndef CPE_CFG_READ_H
#define CPE_CFG_READ_H
#include "cpe/utils/stream.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*basic operations*/
const char * cfg_name(cfg_t cfg);
void * cfg_data(cfg_t cfg);
int cfg_type(cfg_t cfg);
cfg_t cfg_parent(cfg_t cfg);
int cfg_type_is_value(int type);
int cfg_is_value(cfg_t cfg);
cfg_t cfg_find_cfg(cfg_t cfg, const char * path);

int cfg_path_print(write_stream_t stream, cfg_t cfg, cfg_t to);
const char * cfg_path(mem_buffer_t buffer, cfg_t cfg, cfg_t to);

/*get data operation*/
int8_t cfg_as_int8(cfg_t cfg, int8_t dft);
uint8_t cfg_as_uint8(cfg_t cfg, uint8_t dft);
int16_t cfg_as_int16(cfg_t cfg, int16_t dft);
uint16_t cfg_as_uint16(cfg_t cfg, uint16_t dft);
int32_t cfg_as_int32(cfg_t cfg, int32_t dft);
uint32_t cfg_as_uint32(cfg_t cfg, uint32_t dft);
int64_t cfg_as_int64(cfg_t cfg, int64_t dft);
uint64_t cfg_as_uint64(cfg_t cfg, uint64_t dft);
float cfg_as_float(cfg_t cfg, float dft);
double cfg_as_double(cfg_t cfg, double dft);
const char * cfg_as_string(cfg_t cfg, const char * dft);
const char * cfg_as_string_cvt(cfg_t cfg, const char * dft, void * buf, size_t buf_capacity);
const char * cfg_as_string_cvt_buffer(cfg_t cfg, const char * dft, mem_buffer_t buffer);

int cfg_try_as_int8(cfg_t cfg, int8_t * data);
int cfg_try_as_uint8(cfg_t cfg, uint8_t * data);
int cfg_try_as_int16(cfg_t cfg, int16_t * data);
int cfg_try_as_uint16(cfg_t cfg, uint16_t * data);
int cfg_try_as_int32(cfg_t cfg, int32_t * data);
int cfg_try_as_uint32(cfg_t cfg, uint32_t * data);
int cfg_try_as_int64(cfg_t cfg, int64_t * data);
int cfg_try_as_uint64(cfg_t cfg, uint64_t * data);
int cfg_try_as_float(cfg_t cfg, float * data);
int cfg_try_as_double(cfg_t cfg, double * data);

/*get data by path operation*/
int8_t cfg_get_int8(cfg_t cfg, const char * path, int8_t dft);
uint8_t cfg_get_uint8(cfg_t cfg, const char * path, uint8_t dft);
int16_t cfg_get_int16(cfg_t cfg, const char * path, int16_t dft);
uint16_t cfg_get_uint16(cfg_t cfg, const char * path, uint16_t dft);
int32_t cfg_get_int32(cfg_t cfg, const char * path, int32_t dft);
uint32_t cfg_get_uint32(cfg_t cfg, const char * path, uint32_t dft);
int64_t cfg_get_int64(cfg_t cfg, const char * path, int64_t dft);
uint64_t cfg_get_uint64(cfg_t cfg, const char * path, uint64_t dft);
float cfg_get_float(cfg_t cfg, const char * path, float dft);
double cfg_get_double(cfg_t cfg, const char * path, double dft);
const char * cfg_get_string(cfg_t cfg, const char * path, const char * dft);
const char * cfg_get_string_cvt(cfg_t cfg, const char * path, const char * dft, void * buf, size_t buf_capacity);
const char * cfg_get_string_cvt_buffer(cfg_t cfg, const char * path, const char * dft, mem_buffer_t buffer);
    
int cfg_try_get_int8(cfg_t cfg, const char * path, int8_t * data);
int cfg_try_get_uint8(cfg_t cfg, const char * path, uint8_t * data);
int cfg_try_get_int16(cfg_t cfg, const char * path, int16_t * data);
int cfg_try_get_uint16(cfg_t cfg, const char * path, uint16_t * data);
int cfg_try_get_int32(cfg_t cfg, const char * path, int32_t * data);
int cfg_try_get_uint32(cfg_t cfg, const char * path, uint32_t * data);
int cfg_try_get_int64(cfg_t cfg, const char * path, int64_t * data);
int cfg_try_get_uint64(cfg_t cfg, const char * path, uint64_t * data);
int cfg_try_get_float(cfg_t cfg, const char * path, float * data);
int cfg_try_get_double(cfg_t cfg, const char * path, double * data);

/*struct operations*/
int cfg_struct_count(cfg_t cfg);
cfg_t cfg_struct_find_cfg(cfg_t cfg, const char * name);

/*sequence operations*/
int cfg_seq_count(cfg_t cfg);
cfg_t cfg_seq_at(cfg_t cfg, int pos);

/*dump operations*/
void cfg_print(cfg_t cfg, write_stream_t stream, int ident, int level_ident);
void cfg_print_inline(cfg_t cfg, write_stream_t stream);

const char * cfg_dump(cfg_t cfg, mem_buffer_t buffer, int ident, int level_ident);
const char * cfg_dump_inline(cfg_t cfg, mem_buffer_t buffer);

/*child operations*/
int cfg_child_count(cfg_t cfg);
cfg_t cfg_child_only(cfg_t cfg);
void cfg_it_init(cfg_it_t * it, cfg_t cfg);
#define cfg_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*compire operations*/
enum {
    CFG_CMP_POLICY_L_STRUCT_LEAK = 1 << 0
    , CFG_CMP_POLICY_R_STRUCT_LEAK = 1 << 1
};
int cfg_cmp(cfg_t l, cfg_t r, int policy, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif


