#ifndef UI_RUNTIME_RENDER_CMD_UTILS_2D_H
#define UI_RUNTIME_RENDER_CMD_UTILS_2D_H
#include "ui_runtime_render_cmd.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_runtime_render_program_buildin_t
ui_runtime_render_second_color_mix_to_program(ui_runtime_render_second_color_mix_t second_color_mix);

uint8_t runtime_render_cmd_state_and_texture_compatible(
    ui_runtime_render_cmd_t cmd,
    float logic_z,
    ui_transform_t transform,
    ui_cache_res_t texture,
    ui_runtime_render_texture_filter_t filter,
    ui_runtime_render_program_buildin_t program, ui_runtime_render_blend_t blend);
    
/*quad基础接口 */
ui_runtime_render_cmd_t
ui_runtime_render_cmd_quad_create_2d_buildin(
    ui_runtime_render_t render, float logic_z,
    ui_transform_t transform,
    ui_runtime_vertex_v3f_t2f_c4ub_t vertexs,
    ui_cache_res_t texture,
    ui_runtime_render_texture_filter_t filter,
    ui_runtime_render_program_buildin_t program, ui_runtime_render_blend_t blend);

/*quad批量接口 */    
int ui_runtime_render_cmd_quad_batch_append(
    ui_runtime_render_cmd_t * batch_cmd,
    ui_runtime_render_t render, float logic_z,
    ui_transform_t transform,
    ui_runtime_vertex_v3f_t2f_c4ub_t vertexs,
    ui_cache_res_t texture, ui_runtime_render_texture_filter_t filter,
    ui_runtime_render_program_buildin_t program, ui_runtime_render_blend_t blend);

int ui_runtime_render_cmd_quad_batch_commit(
    ui_runtime_render_cmd_t * cmd,
    ui_runtime_render_t render);
    
/*triangle基础接口 */
ui_runtime_render_cmd_t
ui_runtime_render_cmd_triangle_create_2d_buildin(
    ui_runtime_render_t render, float logic_z,
    ui_transform_t transform,
    ui_runtime_vertex_v3f_t2f_c4ub_t vertexs,
    ui_cache_res_t texture,
    ui_runtime_render_texture_filter_t filter,
    ui_runtime_render_program_buildin_t program, ui_runtime_render_blend_t blend);

/*triangle批量接口 */    
int ui_runtime_render_cmd_triangle_batch_append(
    ui_runtime_render_cmd_t * batch_cmd,
    ui_runtime_render_t render, float logic_z,
    ui_transform_t transform,
    ui_runtime_vertex_v3f_t2f_c4ub_t vertexs,
    ui_cache_res_t texture, ui_runtime_render_texture_filter_t filter,
    ui_runtime_render_program_buildin_t program, ui_runtime_render_blend_t blend);

int ui_runtime_render_cmd_triangle_batch_commit(
    ui_runtime_render_cmd_t * cmd,
    ui_runtime_render_t render);
    
#ifdef __cplusplus
}
#endif

#endif
