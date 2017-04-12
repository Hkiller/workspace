#ifndef UI_SPRITE_CAMERA_TRACE_IN_LINE_I_H
#define UI_SPRITE_CAMERA_TRACE_IN_LINE_I_H
#include "ui/sprite_camera/ui_sprite_camera_trace_in_line.h"
#include "ui_sprite_camera_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_camera_trace_in_line {
    ui_sprite_camera_module_t m_module;
};

int ui_sprite_camera_trace_in_line_regist(ui_sprite_camera_module_t module);
void ui_sprite_camera_trace_in_line_unregist(ui_sprite_camera_module_t module);

#ifdef __cplusplus
}
#endif

#endif
