#ifndef PLUGIN_UI_ANIMATION_CONTROL_MOVE_I_H
#define PLUGIN_UI_ANIMATION_CONTROL_MOVE_I_H
#include "render/utils/ui_percent_decorator.h"
#include "render/utils/ui_vector_2.h"
#include "plugin/ui/plugin_ui_anim_control_move.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_anim_control_move {
    plugin_ui_module_t m_module;
    /*config*/
	struct ui_percent_decorator m_cfg_percent_decorator;
    plugin_ui_move_algorithm_t m_cfg_algorithm;
    float m_cfg_take_time;
    uint8_t m_cfg_update_target;
    char * m_cfg_origin;
    char * m_cfg_target;

    /*runtime*/
    ui_vector_2 m_origin;
    ui_vector_2 m_target;
    float m_runing_time;
};

int plugin_ui_anim_control_move_regist(plugin_ui_module_t module);
void plugin_ui_anim_control_move_unregist(plugin_ui_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
