#ifndef UI_SPRITE_CAMERA_SCALE_I_H
#define UI_SPRITE_CAMERA_SCALE_I_H
#include "ui/sprite_camera/ui_sprite_camera_scale.h"
#include "ui_sprite_camera_module_i.h"
#include "ui_sprite_camera_updator.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_camera_scale {
    ui_sprite_camera_module_t m_module;
    struct ui_sprite_camera_updator m_updator;
};

int ui_sprite_camera_scale_regist(ui_sprite_camera_module_t module);
void ui_sprite_camera_scale_unregist(ui_sprite_camera_module_t module);

#ifdef __cplusplus
}
#endif

#endif
