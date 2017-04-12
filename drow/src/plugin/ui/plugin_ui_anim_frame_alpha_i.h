#ifndef PLUGIN_UI_ANIM_FRAME_ALPHA_I_H
#define PLUGIN_UI_ANIM_FRAME_ALPHA_I_H
#include "render/utils/ui_percent_decorator.h"
#include "render/utils/ui_vector_2.h"
#include "plugin/ui/plugin_ui_anim_frame_alpha.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_anim_frame_alpha {
    /*config*/
	struct ui_percent_decorator m_percent_decorator;
    float m_cfg_target;
    float m_cfg_take_time;
    
    /*runtime*/
    float m_runing_time;
};

int plugin_ui_anim_frame_alpha_regist(plugin_ui_module_t module);
void plugin_ui_anim_frame_alpha_unregist(plugin_ui_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
