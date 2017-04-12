#ifndef UI_RUNTIME_RENDER_MATERIAL_UTILS_H
#define UI_RUNTIME_RENDER_MATERIAL_UTILS_H
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_runtime_render_material_t
ui_runtime_render_material_create_from_program(
    ui_runtime_render_t render,
    ui_runtime_render_program_state_t program_state);

ui_runtime_render_material_t
ui_runtime_render_material_create_from_buildin_program(
    ui_runtime_render_t render,
    ui_runtime_render_program_buildin_t buildin_program);

ui_runtime_render_technique_t
ui_runtime_render_technique_create_from_program(
    ui_runtime_render_material_t material,
    ui_runtime_render_program_state_t program_state);

int ui_runtime_render_material_set_unifs(ui_runtime_render_material_t material, ui_transform_t mv);
int ui_runtime_render_material_set_attrs(ui_runtime_render_material_t material, ui_runtime_render_buff_type_t buff_type);
int ui_runtime_render_material_set_unifs_and_attrs(ui_runtime_render_material_t material, ui_transform_t mv, ui_runtime_render_buff_type_t buff_type);
    
int ui_runtime_render_material_set_texture_dft(
    ui_runtime_render_material_t material,
    ui_cache_res_t res,
    ui_runtime_render_texture_filter_t min_filter,
    ui_runtime_render_texture_filter_t mag_filter,
    ui_runtime_render_texture_wrapping_t wrap_s,
    ui_runtime_render_texture_wrapping_t wrap_t,
    uint8_t texture_idx);
                                           
#ifdef __cplusplus
}
#endif

#endif
