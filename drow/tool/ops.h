#ifndef UI_MODEL_TOO_OPS_H
#define UI_MODEL_TOO_OPS_H
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "render/model/ui_data_mgr.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "render/cache/ui_cache_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int do_manip_model(
    gd_app_context_t app, ui_data_mgr_t data_mgr, const char * op_script, error_monitor_t em);

int do_repaire_model(
    gd_app_context_t app, ui_data_mgr_t data_mgr, ui_ed_mgr_t ed_mgr, error_monitor_t em);
    
int do_convert_model(
    gd_app_context_t app, const char * root, const char * to, const char * format,
    uint32_t texture_limit_width, uint32_t texture_limit_height, 
    const char * texture_compress, const char * tinypng_accounts,
    const char * * languages, uint32_t language_count,
    const char * pack_way,
    uint8_t is_full,
    error_monitor_t em);

int do_cocos_module_import(
    gd_app_context_t app, ui_data_mgr_t data_mgr,
    const char * to_module,
    const char * plist, const char * pic);

int do_cocos_effect_import(
    gd_app_context_t app, ui_data_mgr_t data_mgr,
    const char * to_effect, const char * to_module,
    const char * plist, const char * pic, uint8_t frame_duration,
    const char * frame_position, const char * frame_order);

int do_cocos_particle_import(
    gd_app_context_t app, ui_data_mgr_t data_mgr,
    const char * to_particle,
    const char * plist, const char * pic);

#ifdef __cplusplus
}
#endif

#endif
