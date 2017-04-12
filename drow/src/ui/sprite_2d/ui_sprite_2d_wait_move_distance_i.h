#ifndef UI_SPRITE_2D_WAIT_MOVE_DISTANCE_I_H
#define UI_SPRITE_2D_WAIT_MOVE_DISTANCE_I_H
#include "render/utils/ui_vector_2.h"
#include "ui_sprite_2d_module_i.h"
#include "ui/sprite_2d/ui_sprite_2d_wait_move_distance.h"

#ifdef __cplusplus
extern "C" {
#endif
	
struct ui_sprite_2d_wait_move_distance {
	ui_sprite_2d_module_t m_module;
    char * m_cfg_distance;

    float m_distance;
    float m_moved_distance;
    ui_vector_2 m_pre_pos;
};

int ui_sprite_2d_wait_move_distance_regist(ui_sprite_2d_module_t module);
void ui_sprite_2d_wait_move_distance_unregist(ui_sprite_2d_module_t module);

#ifdef __cplusplus
}
#endif

#endif
