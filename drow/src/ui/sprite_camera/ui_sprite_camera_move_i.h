#ifndef UI_SPRITE_CAMERA_MOVE_I_H
#define UI_SPRITE_CAMERA_MOVE_I_H
#include "ui/sprite_camera/ui_sprite_camera_move.h"
#include "ui_sprite_camera_module_i.h"
#include "ui_sprite_camera_updator.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_camera_move {
    ui_sprite_camera_module_t m_module;
    struct ui_sprite_camera_updator m_updator;

    ui_vector_2 m_pos_in_world;
    ui_vector_2 m_pos_in_screen;
};

int ui_sprite_camera_move_regist(ui_sprite_camera_module_t module);
void ui_sprite_camera_move_unregist(ui_sprite_camera_module_t module);

#ifdef __cplusplus
}
#endif

#endif
