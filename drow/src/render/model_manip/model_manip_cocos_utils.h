#ifndef UI_MODEL_MANIP_COCOS_UTILS_H
#define UI_MODEL_MANIP_COCOS_UTILS_H
#include "cpe/cfg/cfg.h"
#include "render/model/ui_data_module.h"
#include "render/model/ui_data_action.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "render/model_ed/ui_ed_obj.h"
#include "render/model_ed/ui_ed_src.h"
#include "render/model_manip/model_manip_cocos.h"
#include "render/model_manip/model_manip_types.h"

UI_IMG_BLOCK * cocos_build_img_block(
    cfg_t frame_cfg, ui_ed_obj_t module_obj, uint32_t img_id, const char * pic, error_monitor_t em);

UI_ACTOR_FRAME * cocos_build_actor_frame(
    cfg_t frame_cfg, ui_ed_obj_t layer_obj, uint32_t module_id, UI_IMG_BLOCK const * img_block,
    uint32_t idx, uint32_t frame_duration, ui_manip_action_import_frame_position_t frame_position, error_monitor_t em);

#endif
