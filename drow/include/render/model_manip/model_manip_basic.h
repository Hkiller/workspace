#ifndef UI_MODEL_MANIP_BASIC_H
#define UI_MODEL_MANIP_BASIC_H
#include "model_manip_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int ui_model_import_module(
    ui_ed_mgr_t ed_mgr, ui_cache_manager_t cache_mgr,
    const char * to_module, const char * base_dir, const char * * srcs, uint32_t src_count,
    error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
