#ifndef UI_SPRITE_2D_SCALE_I_H
#define UI_SPRITE_2D_SCALE_I_H
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_percent_decorator.h"
#include "ui_sprite_2d_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif
	
struct ui_sprite_2d_scale {
	ui_sprite_2d_module_t m_module;
    char * m_cfg_target_scale_x;
    char * m_cfg_target_scale_y;
	char * m_cfg_duration;
    char * m_cfg_step;
    struct ui_percent_decorator m_cfg_decorator;

	ui_vector_2 m_target_scale;
	ui_vector_2	m_origin_scale;
    float m_runing_time;
    ui_vector_2 m_work_duration;
};

int ui_sprite_2d_scale_regist(ui_sprite_2d_module_t module);
void ui_sprite_2d_scale_unregist(ui_sprite_2d_module_t module);

#ifdef __cplusplus
}
#endif

#endif
