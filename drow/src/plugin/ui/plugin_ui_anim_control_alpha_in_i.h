#ifndef PLUGIN_UI_ANIMATION_ALPHA_IN_I_H
#define PLUGIN_UI_ANIMATION_ALPHA_IN_I_H
#include "render/utils/ui_percent_decorator.h"
#include "plugin/ui/plugin_ui_anim_control_alpha_in.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_anim_control_alpha_in {
    plugin_ui_module_t m_module;
	struct ui_percent_decorator m_cfg_percent_decorator;
    float m_cfg_origin;
    float m_cfg_take_time;
    uint16_t m_cfg_frames;
    
    float m_origin;
    float m_target;
    float m_runing_time;
};

int plugin_ui_anim_control_alpha_in_regist(plugin_ui_module_t module);
void plugin_ui_anim_control_alpha_in_unregist(plugin_ui_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
