#ifndef UI_RUNTIME_RENDER_CMD_TRIANGLES_H
#define UI_RUNTIME_RENDER_CMD_TRIANGLES_H
#include "ui_runtime_render_cmd.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_runtime_render_cmd_t    
ui_runtime_render_cmd_triangles_create(
    ui_runtime_render_t render,
    float global_order,
    ui_runtime_render_material_t material,
    struct ui_runtime_render_buff_use vertexs,
    struct ui_runtime_render_buff_use indexes,
    ui_transform_t mv, uint8_t is_3d);

ui_runtime_render_cmd_t
ui_runtime_render_cmd_quad_create(
    ui_runtime_render_t render,
    float logic_z,
    ui_runtime_render_material_t material,
    struct ui_runtime_render_buff_use vertexs,
    ui_transform_t mv, uint8_t is_3d);

ui_runtime_render_cmd_t
ui_runtime_render_cmd_triangle_create(
    ui_runtime_render_t render,
    float logic_z,
    ui_runtime_render_material_t material,
    struct ui_runtime_render_buff_use vertexs,
    ui_transform_t mv, uint8_t is_3d);

#ifdef __cplusplus
}
#endif

#endif
