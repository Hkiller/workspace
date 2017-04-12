#ifndef CPE_CFG_MANAGE_H
#define CPE_CFG_MANAGE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/stream.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/zip/zip_types.h"
#include "cpe/vfs/vfs_types.h"
#include "cfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

cfg_t cfg_create(mem_allocrator_t alloc);
void cfg_free(cfg_t cfg);

/*struct operation*/
cfg_t cfg_struct_add_struct(cfg_t s, const char * name, cfg_policy_t policy);
cfg_t cfg_struct_add_seq(cfg_t s, const char * name, cfg_policy_t policy);
cfg_t cfg_struct_add_string(cfg_t s, const char * name, const char * value, cfg_policy_t policy);
cfg_t cfg_struct_add_string_len(cfg_t s, const char * name, const char * value, size_t len, cfg_policy_t policy);
cfg_t cfg_struct_add_int8(cfg_t s, const char * name, int8_t v, cfg_policy_t policy);
cfg_t cfg_struct_add_uint8(cfg_t s, const char * name, uint8_t v, cfg_policy_t policy);
cfg_t cfg_struct_add_int16(cfg_t s, const char * name, int16_t v, cfg_policy_t policy);
cfg_t cfg_struct_add_uint16(cfg_t s, const char * name, uint16_t v, cfg_policy_t policy);
cfg_t cfg_struct_add_int32(cfg_t s, const char * name, int32_t v, cfg_policy_t policy);
cfg_t cfg_struct_add_uint32(cfg_t s, const char * name, uint32_t v, cfg_policy_t policy);
cfg_t cfg_struct_add_int64(cfg_t s, const char * name, int64_t v, cfg_policy_t policy);
cfg_t cfg_struct_add_uint64(cfg_t s, const char * name, uint64_t v, cfg_policy_t policy);
cfg_t cfg_struct_add_float(cfg_t s, const char * name, float v, cfg_policy_t policy);
cfg_t cfg_struct_add_double(cfg_t s, const char * name, double v, cfg_policy_t policy);
cfg_t cfg_struct_add_value(cfg_t s, const char * name, int typeId, cfg_policy_t policy);
cfg_t cfg_struct_add_value_from_string(cfg_t s, const char * name, int typeId, const char * value, cfg_policy_t policy);
cfg_t cfg_struct_add_value_from_string_auto(cfg_t s, const char * name, const char * value, cfg_policy_t policy);
cfg_t cfg_struct_add_value_from_binary(cfg_t s, const char * name, int typeId, const void * value, cfg_policy_t policy);

/*seq operation*/
cfg_t cfg_seq_add_struct(cfg_t s);
cfg_t cfg_seq_add_seq(cfg_t s);
cfg_t cfg_seq_add_string(cfg_t s, const char * value);
cfg_t cfg_seq_add_string_len(cfg_t s, const char * value, size_t len);
cfg_t cfg_seq_add_int8(cfg_t s, int8_t v);
cfg_t cfg_seq_add_uint8(cfg_t s, uint8_t v);
cfg_t cfg_seq_add_int16(cfg_t s, int16_t v);
cfg_t cfg_seq_add_uint16(cfg_t s, uint16_t v);
cfg_t cfg_seq_add_int32(cfg_t s, int32_t v);
cfg_t cfg_seq_add_uint32(cfg_t s, uint32_t v);
cfg_t cfg_seq_add_int64(cfg_t s, int64_t v);
cfg_t cfg_seq_add_uint64(cfg_t s, uint64_t v);
cfg_t cfg_seq_add_float(cfg_t s, float v);
cfg_t cfg_seq_add_double(cfg_t s, double v);
cfg_t cfg_seq_add_value(cfg_t s, int typeId);
cfg_t cfg_seq_add_value_from_string(cfg_t s, int typeId, const char * value);
cfg_t cfg_seq_add_value_from_string_auto(cfg_t s, const char * value);
cfg_t cfg_seq_add_value_from_binary(cfg_t s, int typeId, const void * value);

cfg_t cfg_add_struct(cfg_t c, const char * path, error_monitor_t em);
cfg_t cfg_add_seq(cfg_t c, const char * path, error_monitor_t em);
cfg_t cfg_add_string(cfg_t c, const char * path, const char * value, error_monitor_t em);
cfg_t cfg_add_int8(cfg_t c, const char * path, int8_t v, error_monitor_t em);
cfg_t cfg_add_uint8(cfg_t c, const char * path, uint8_t v, error_monitor_t em);
cfg_t cfg_add_int16(cfg_t c, const char * path, int16_t v, error_monitor_t em);
cfg_t cfg_add_uint16(cfg_t c, const char * path, uint16_t v, error_monitor_t em);
cfg_t cfg_add_int32(cfg_t c, const char * path, int32_t v, error_monitor_t em);
cfg_t cfg_add_uint32(cfg_t c, const char * path, uint32_t v, error_monitor_t em);
cfg_t cfg_add_int64(cfg_t c, const char * path, int64_t v, error_monitor_t em);
cfg_t cfg_add_uint64(cfg_t c, const char * path, uint64_t v, error_monitor_t em);
cfg_t cfg_add_float(cfg_t c, const char * path, float v, error_monitor_t em);
cfg_t cfg_add_double(cfg_t c, const char * path, double v, error_monitor_t em);
cfg_t cfg_add_value_from_string(cfg_t c, const char * path, int typeId, const char * value, error_monitor_t em);
cfg_t cfg_add_value_from_string_auto(cfg_t c, const char * path, const char * value, error_monitor_t em);
cfg_t cfg_add_value_from_binary(cfg_t c, const char * path, int typeId, const void * value, error_monitor_t em);

int cfg_merge(cfg_t cfg, cfg_t input, cfg_policy_t policy, error_monitor_t em);

/*dir*/
int cfg_read_dir(cfg_t cfg, vfs_mgr_t vfs, const char * path, cfg_policy_t policy, error_monitor_t em, mem_allocrator_t talloc);
    
/*yaml*/
int cfg_yaml_read(cfg_t cfg, read_stream_t stream, cfg_policy_t policy, error_monitor_t em);
int cfg_yaml_read_with_name(cfg_t cfg, const char * name, read_stream_t stream, cfg_policy_t policy, error_monitor_t em);
int cfg_yaml_read_file(cfg_t cfg, vfs_mgr_t vfs, const char * path, cfg_policy_t policy, error_monitor_t em);
int cfg_yaml_read_file_with_name(cfg_t cfg, const char * name, vfs_mgr_t vfs, const char * path, cfg_policy_t policy, error_monitor_t em);
    
int cfg_yaml_write(write_stream_t stream, cfg_t cfg, error_monitor_t em);
int cfg_yaml_write_file(cfg_t cfg, vfs_mgr_t vfs, const char * path, error_monitor_t em);
    
/*bin*/
int cfg_bin_write(void * result, size_t result_capacity, cfg_t cfg, error_monitor_t em);
int cfg_bin_write_to_buffer(mem_buffer_t result, cfg_t cfg, error_monitor_t em);
int cfg_bin_write_file(cfg_t cfg, vfs_mgr_t vfs, const char * path, error_monitor_t em);

int cfg_bin_read(cfg_t cfg, void const  * input, size_t input_len, error_monitor_t em);
int cfg_bin_read_with_name(cfg_t cfg, const char * name, void const  * input, size_t input_len, error_monitor_t em);
int cfg_bin_read_file(cfg_t cfg, vfs_mgr_t vfs, const char * path, error_monitor_t em);

/*json*/
int cfg_json_read(cfg_t cfg, read_stream_t stream, cfg_policy_t policy, error_monitor_t em);
int cfg_json_read_with_name(cfg_t cfg, const char * name, read_stream_t stream, cfg_policy_t policy, error_monitor_t em);
int cfg_json_read_file(cfg_t cfg, vfs_mgr_t vfs, const char * path, cfg_policy_t policy, error_monitor_t em);

int cfg_apply_modify(cfg_t cfg, cfg_t modify_info, error_monitor_t em);
int cfg_apply_modify_seq(cfg_t cfg, cfg_t modify_info, error_monitor_t em);

/*zip*/
int cfg_read_zip_dir(cfg_t cfg, cpe_unzip_dir_t d, cfg_policy_t policy, error_monitor_t em, mem_allocrator_t talloc);
int cfg_read_zip_file(cfg_t cfg, cpe_unzip_file_t zf, cfg_policy_t policy, error_monitor_t em);
int cfg_read_zip_bin_file(cfg_t cfg, cpe_unzip_file_t zf, cfg_policy_t policy, error_monitor_t em);

    
#ifdef __cplusplus
}
#endif

#endif


