#ifndef PLUGIN_UI_ANIMATION_CONTROL_BASIC_I_H
#define PLUGIN_UI_ANIMATION_CONTROL_BASIC_I_H
#include "render/utils/ui_percent_decorator.h"
#include "render/utils/ui_vector_2.h"
#include "plugin/ui/plugin_ui_anim_control_anim.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_anim_control_anim {
    /*config*/
	struct ui_percent_decorator m_percent_decorator;
    ui_data_control_anim_t m_anim_data;
    float m_frame_time;
    uint32_t m_total_frame;
    uint32_t m_loop_count;
    uint8_t m_soft;
    
    /*runtime*/
    float m_runing_time;
    int32_t m_cur_frame;
    
    ui_data_control_anim_frame_t m_cur_trans_frame;
    ui_data_control_anim_frame_t m_next_trans_frame;
    ui_data_control_anim_frame_t m_cur_scale_frame;
    ui_data_control_anim_frame_t m_next_scale_frame;
    ui_data_control_anim_frame_t m_cur_alpha_frame;
    ui_data_control_anim_frame_t m_next_alpha_frame;
    ui_data_control_anim_frame_t m_cur_color_frame;
    ui_data_control_anim_frame_t m_next_color_frame;
    ui_data_control_anim_frame_t m_cur_angle_frame;
    ui_data_control_anim_frame_t m_next_angle_frame;
};

int plugin_ui_anim_control_anim_regist(plugin_ui_module_t module);
void plugin_ui_anim_control_anim_unregist(plugin_ui_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
