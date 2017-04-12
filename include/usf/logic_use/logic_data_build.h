#ifndef USF_LOGIC_USE_DATA_BUILD_H
#define USF_LOGIC_USE_DATA_BUILD_H
#include "cpe/utils/error.h"
#include "logic_use_types.h"

#ifdef __cplusplus
extern "C" {
#endif

logic_data_t logic_data_build(logic_context_t context, LPDRMETA meta, cfg_t cfg, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif

