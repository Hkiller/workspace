#ifndef PLUGIN_UI_ANIMATION_FRAME_MOVE_I_H
#define PLUGIN_UI_ANIMATION_FRAME_MOVE_I_H
#include "render/utils/ui_percent_decorator.h"
#include "plugin/ui/plugin_ui_anim_control_frame_move.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_anim_control_frame_move {
    plugin_ui_module_t m_module;
	struct ui_percent_decorator m_cfg_percent_decorator;
    struct ui_vector_2 m_cfg_target_pos;
    float m_cfg_take_time;
    
    struct ui_vector_2 m_origin_pos;
    struct ui_vector_2 m_target_pos;
    float m_runing_time;
};

int plugin_ui_anim_control_frame_move_regist(plugin_ui_module_t module);
void plugin_ui_anim_control_frame_move_unregist(plugin_ui_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
