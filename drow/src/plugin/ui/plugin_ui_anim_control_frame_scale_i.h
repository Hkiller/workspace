#ifndef PLUGIN_UI_ANIMATION_FRAME_SCALE_I_H
#define PLUGIN_UI_ANIMATION_FRAME_SCALE_I_H
#include "render/utils/ui_percent_decorator.h"
#include "plugin/ui/plugin_ui_anim_control_frame_scale.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_anim_control_frame_scale {
    plugin_ui_module_t m_module;
	struct ui_percent_decorator m_cfg_percent_decorator;
    struct ui_vector_2 m_cfg_target_scale;
    float m_cfg_take_time;
    
    struct ui_vector_2 m_origin_scale;
    struct ui_vector_2 m_target_scale;
    float m_runing_time;
};

int plugin_ui_anim_control_frame_scale_regist(plugin_ui_module_t module);
void plugin_ui_anim_control_frame_scale_unregist(plugin_ui_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
