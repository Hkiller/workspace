#ifndef UI_SPRITE_CAMERA_CONTAIN_I_H
#define UI_SPRITE_CAMERA_CONTAIN_I_H
#include "ui/sprite_camera/ui_sprite_camera_contain.h"
#include "ui_sprite_camera_module_i.h"
#include "ui_sprite_camera_updator.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_camera_contain {
    ui_sprite_camera_module_t m_module;
    struct ui_sprite_camera_updator m_updator;

    uint32_t m_group_id;
    char m_group_name[64];

    ui_rect m_screen_rect;
    float m_max_scale;
    float m_best_scale;
};

int ui_sprite_camera_contain_regist(ui_sprite_camera_module_t module);
void ui_sprite_camera_contain_unregist(ui_sprite_camera_module_t module);

#ifdef __cplusplus
}
#endif

#endif
