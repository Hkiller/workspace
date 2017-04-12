#ifndef PLUGIN_UI_ANIMATION_SCALE_IN_I_H
#define PLUGIN_UI_ANIMATION_SCALE_IN_I_H
#include "render/utils/ui_percent_decorator.h"
#include "plugin/ui/plugin_ui_anim_control_scale_in.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_anim_control_scale_in {
    plugin_ui_module_t m_module;
	struct ui_percent_decorator m_cfg_percent_decorator;
    struct ui_vector_2 m_cfg_origin_scale;
    float m_cfg_take_time;
    uint16_t m_cfg_frames;
    
    struct ui_vector_2 m_origin_scale;
    struct ui_vector_2 m_target_scale;
    float m_runing_time;
};

int plugin_ui_anim_control_scale_in_regist(plugin_ui_module_t module);
void plugin_ui_anim_control_scale_in_unregist(plugin_ui_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
