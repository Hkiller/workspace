#ifndef DROW_PLUGIN_BASICANIM_UTILS_H
#define DROW_PLUGIN_BASICANIM_UTILS_H
#include "plugin_basicanim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_runtime_render_cmd_t
plugin_basicanim_render_draw_color(
    ui_runtime_render_t render, float logic_z,
    ui_rect_t clip_rect, ui_transform_t transform,
    ui_rect_t rect, ui_color_t color);

void plugin_basicanim_render_draw_rect(
    ui_runtime_render_t render, ui_runtime_render_cmd_t * batch_cmd, ui_rect_t clip_rect,
    ui_transform_t transform, ui_runtime_render_second_color_t second_color,
    ui_cache_res_t texture, ui_rect_t texture_rect, ui_runtime_render_texture_filter_t texture_filter);

int plugin_basicanim_render_draw_frame(
    ui_runtime_render_t render, ui_runtime_render_cmd_t * batch_cmd, ui_rect_t clip_rect,
    ui_transform_t transform,
    ui_data_frame_t frame,
    ui_runtime_render_second_color_t second_color,
    error_monitor_t em);
    
#ifdef __cplusplus
}
#endif

#endif
