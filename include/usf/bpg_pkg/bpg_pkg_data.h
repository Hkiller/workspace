#ifndef USF_BPG_PKG_DATA_H
#define USF_BPG_PKG_DATA_H
#include "cpe/utils/hash_string.h"
#include "cpe/dr/dr_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "bpg_pkg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void * bpg_pkg_body(dp_req_t body);
size_t bpg_pkg_body_size(dp_req_t body);
size_t bpg_pkg_body_capacity(dp_req_t body);

LPDRMETALIB bpg_pkg_data_meta_lib(dp_req_t body);

/*pkg operations*/
uint32_t bpg_pkg_cmd(dp_req_t body);
void bpg_pkg_set_cmd(dp_req_t body, uint32_t cmd);

uint32_t bpg_pkg_errno(dp_req_t body);
void bpg_pkg_set_errno(dp_req_t body, uint32_t en);

uint64_t bpg_pkg_client_id(dp_req_t body);
void bpg_pkg_set_client_id(dp_req_t body, uint64_t client_id);

uint32_t bpg_pkg_sn(dp_req_t body);
void bpg_pkg_set_sn(dp_req_t body, uint32_t sn);

uint32_t bpg_pkg_flags(dp_req_t body);
void bpg_pkg_set_flags(dp_req_t body, uint32_t flags);

int bpg_pkg_flag_enable(dp_req_t body, bpg_pkg_flag_t flag);
void bpg_pkg_flag_set_enable(dp_req_t body, bpg_pkg_flag_t flag, int is_enable);

int32_t bpg_pkg_append_info_count(dp_req_t body);
bpg_pkg_append_info_t bpg_pkg_append_info_at(dp_req_t body, int32_t pos);

uint32_t bpg_pkg_append_info_id(bpg_pkg_append_info_t append_info);
uint32_t bpg_pkg_append_info_size(bpg_pkg_append_info_t append_info);

/*main data ops*/
int bpg_pkg_set_main_data(dp_req_t body, const void * buf, size_t size, error_monitor_t em);
void * bpg_pkg_main_data(dp_req_t body);
size_t bpg_pkg_main_data_len(dp_req_t body);
LPDRMETA bpg_pkg_main_data_meta(dp_req_t body, error_monitor_t em);

/*append data ops*/
int bpg_pkg_add_append_data(dp_req_t body, LPDRMETA meta, const void * buf, size_t size, error_monitor_t em);
size_t bpg_pkg_append_data_len(dp_req_t body, bpg_pkg_append_info_t append_info);
void * bpg_pkg_append_data(dp_req_t body, bpg_pkg_append_info_t append_info);
LPDRMETA bpg_pkg_append_data_meta(dp_req_t body, bpg_pkg_append_info_t append_info, error_monitor_t em);

void bpg_pkg_init(dp_req_t body);
void bpg_pkg_clear(dp_req_t body);

const char * bpg_pkg_dump(dp_req_t req, mem_buffer_t buffer);
int bpg_pkg_build_from_cfg(dp_req_t req, cfg_t cfg, error_monitor_t em);

bpg_pkg_debug_level_t bpg_pkg_debug_level(dp_req_t body);

dr_cvt_result_t
bpg_pkg_encode(
    dp_req_t body,
    void * output, size_t * output_capacity,
    error_monitor_t em, int debug);

dr_cvt_result_t
bpg_pkg_decode(
    dp_req_t body,
    const void * input, size_t * input_capacity,
    error_monitor_t em, int debug);

dr_cvt_result_t
bpg_pkg_encode_data(
    dp_req_t body,
    void * output, size_t * output_capacity,
    error_monitor_t em, int debug);

dr_cvt_result_t
bpg_pkg_decode_data(
    dp_req_t body,
    const void * input, size_t * input_capacity,
    error_monitor_t em, int debug);

#ifdef __cplusplus
}
#endif

#endif
