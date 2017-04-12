#ifndef UI_SPRITE_2D_TRACK_ANGLE_I_H
#define UI_SPRITE_2D_TRACK_ANGLE_I_H
#include "render/utils/ui_vector_2.h"
#include "ui_sprite_2d_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_2d_track_angle_pos {    
    ui_vector_2 m_pos;
    float m_delta;
};

struct ui_sprite_2d_track_angle {
	ui_sprite_2d_module_t m_module;
    float m_check_span;

    struct ui_sprite_2d_track_angle_pos m_moving_poses[100];
    float m_total_time;
    struct ui_sprite_2d_track_angle_pos * m_moving_poses_wp;
    struct ui_sprite_2d_track_angle_pos * m_moving_poses_rp;
};

int ui_sprite_2d_track_angle_regist(ui_sprite_2d_module_t module);
void ui_sprite_2d_track_angle_unregist(ui_sprite_2d_module_t module);

#ifdef __cplusplus
}
#endif

#endif
