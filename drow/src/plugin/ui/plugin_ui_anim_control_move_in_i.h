#ifndef PLUGIN_UI_ANIMATION_MOVE_IN_I_H
#define PLUGIN_UI_ANIMATION_MOVE_IN_I_H
#include "render/utils/ui_percent_decorator.h"
#include "render/utils/ui_vector_2.h"
#include "plugin/ui/plugin_ui_anim_control_move_in.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_anim_control_move_in {
    plugin_ui_module_t m_module;
	struct ui_percent_decorator m_cfg_percent_decorator;
    plugin_ui_control_move_pos_t m_cfg_origin;
    float m_cfg_take_time;
	float m_cfg_begin_at;
    
    ui_vector_2 m_origin;
    ui_vector_2 m_target;
    float m_runing_time;
};

int plugin_ui_anim_control_move_in_regist(plugin_ui_module_t module);
void plugin_ui_anim_control_move_in_unregist(plugin_ui_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
