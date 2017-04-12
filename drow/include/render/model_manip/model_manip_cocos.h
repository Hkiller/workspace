#ifndef UI_MODEL_MANIP_COCOS_H
#define UI_MODEL_MANIP_COCOS_H
#include "model_manip_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int ui_model_import_cocos_module(
    ui_ed_mgr_t ed_mgr, const char * module_path,
    const char * plist, const char * pic, error_monitor_t em);

int ui_model_import_cocos_action(
    ui_ed_mgr_t ed_mgr, const char * action_path, const char * module_path,
    const char * plist, const char * pic,
    uint32_t frame_duration,
    ui_manip_action_import_frame_position_t frame_position,
    ui_manip_action_import_frame_order_t frame_order,
    error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
