#ifndef UI_SPRITE_2D_WAIT_STOP_I_H
#define UI_SPRITE_2D_WAIT_STOP_I_H
#include "render/utils/ui_vector_2.h"
#include "ui_sprite_2d_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif
	
struct ui_sprite_2d_wait_stop {
	ui_sprite_2d_module_t m_module;
    float m_threshold;

    float m_stoped_time;
    ui_vector_2 m_pre_pos;
};

int ui_sprite_2d_wait_stop_regist(ui_sprite_2d_module_t module);
void ui_sprite_2d_wait_stop_unregist(ui_sprite_2d_module_t module);

#ifdef __cplusplus
}
#endif

#endif
