#ifndef UI_SPRITE_CAMERA_WAIT_STOP_I_H
#define UI_SPRITE_CAMERA_WAIT_STOP_I_H
#include "ui/sprite_camera/ui_sprite_camera_wait_stop.h"
#include "ui_sprite_camera_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_camera_wait_stop {
    ui_sprite_camera_module_t m_module;
};

int ui_sprite_camera_wait_stop_regist(ui_sprite_camera_module_t module);
void ui_sprite_camera_wait_stop_unregist(ui_sprite_camera_module_t module);

#ifdef __cplusplus
}
#endif

#endif
