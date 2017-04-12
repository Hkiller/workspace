#ifndef USF_BPG_PKG_TYPES_H
#define USF_BPG_PKG_TYPES_H
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_types.h"
#include "gd/app/app_context.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bpg_pkg_manage * bpg_pkg_manage_t;
typedef struct bpg_pkg * bpg_pkg_t;
typedef struct bpg_pkg_append_info * bpg_pkg_append_info_t;
typedef struct bpg_pkg_dsp * bpg_pkg_dsp_t;

#define BPG_BASEPKG_LIB_NAME "basepkg"
#define BPG_BASEPKG_META_NAME "basepkg"

#define BPG_INVALID_CONNECTION_ID ((int32_t)-1)

typedef enum bpg_pkg_flag {
    bpg_pkg_flag_oneway = 2
} bpg_pkg_flag_t;

typedef enum bpg_pkg_debug_level {
    bpg_pkg_debug_none
    , bpg_pkg_debug_summary
    , bpg_pkg_debug_detail
    , bpg_pkg_debug_progress
} bpg_pkg_debug_level_t;

#ifdef __cplusplus
}
#endif

#endif
