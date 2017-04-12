#ifndef UI_SPRITE_2D_WAIT_SWITCHBACK_I_H
#define UI_SPRITE_2D_WAIT_SWITCHBACK_I_H
#include "render/utils/ui_vector_2.h"
#include "ui_sprite_2d_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif
	
struct ui_sprite_2d_wait_switchback {
	ui_sprite_2d_module_t m_module;
	uint8_t m_process_x;
	uint8_t m_process_y;
    uint8_t m_track_pos;

    int8_t m_moving_x;
    int8_t m_moving_y;
    ui_vector_2 m_pre_pos;
};

int ui_sprite_2d_wait_switchback_regist(ui_sprite_2d_module_t module);
void ui_sprite_2d_wait_switchback_unregist(ui_sprite_2d_module_t module);

#ifdef __cplusplus
}
#endif

#endif
