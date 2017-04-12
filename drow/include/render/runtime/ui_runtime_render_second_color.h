#ifndef UI_RUNTIME_RENDER_SECOND_COLOR_H
#define UI_RUNTIME_RENDER_SECOND_COLOR_H
#include "render/utils/ui_color.h"
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif
    
struct ui_runtime_render_second_color {
    ui_runtime_render_second_color_mix_t m_mix;
    ui_color m_color;
};

void ui_runtime_render_second_color_mix(ui_runtime_render_second_color_t second_color, ui_color_t to_color);
    
#ifdef __cplusplus
}
#endif

#endif
