#ifndef UI_SPRITE_CAMERA_FOLLOW_I_H
#define UI_SPRITE_CAMERA_FOLLOW_I_H
#include "ui/sprite_camera/ui_sprite_camera_follow.h"
#include "ui_sprite_camera_updator.h"
#include "ui_sprite_camera_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_camera_follow {
    ui_sprite_camera_module_t m_module;
    struct ui_sprite_camera_updator m_updator;

    uint32_t m_follow_entity_id;
    char m_follow_entity_name[64];
    uint32_t m_follow_entity_pos;

    ui_vector_2 m_follow_pos_in_screen;
    float m_best_scale;

    ui_rect m_screen_rect;
};

int ui_sprite_camera_follow_regist(ui_sprite_camera_module_t module);
void ui_sprite_camera_follow_unregist(ui_sprite_camera_module_t module);

#ifdef __cplusplus
}
#endif

#endif
