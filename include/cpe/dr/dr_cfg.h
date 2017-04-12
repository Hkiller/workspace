#ifndef CPE_DR_CFG_H
#define CPE_DR_CFG_H
#include "cpe/utils/error.h"
#include "cpe/cfg/cfg_types.h"
#include "dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DR_CFG_READ_CHECK_NOT_EXIST_ATTR 1

int dr_cfg_read(
    void * result,
    size_t capacity,
    cfg_t cfg,
    LPDRMETA meta,
    int policy,
    error_monitor_t em);

int dr_cfg_write(
    cfg_t cfg,
    const void * data,
    LPDRMETA meta,
    error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
