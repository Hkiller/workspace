#ifndef CPE_PLIST_CFG_H
#define CPE_PLIST_CFG_H
#include "cpe/utils/stream.h"
#include "cpe/utils/error.h"
#include "cpe/cfg/cfg_types.h"
#include "plist_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int plist_cfg_read(cfg_t cfg, read_stream_t stream, cfg_policy_t policy, error_monitor_t em);
int plist_cfg_write(write_stream_t stream, cfg_t cfg, error_monitor_t em);

cfg_t plist_cfg_load_dict_from_file(const char * file_path, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
