#ifndef UI_RUNTIME_RENDER_PROGRAM_STATE_H
#define UI_RUNTIME_RENDER_PROGRAM_STATE_H
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_runtime_render_program_state_t
ui_runtime_render_program_state_create(ui_runtime_render_t render, ui_runtime_render_program_t program);

ui_runtime_render_program_state_t
ui_runtime_render_program_state_clone(ui_runtime_render_program_state_t proto);
    
ui_runtime_render_program_state_t
ui_runtime_render_program_state_create_by_buildin_program(ui_runtime_render_t render, ui_runtime_render_program_buildin_t buildin);

void ui_runtime_render_program_state_free(ui_runtime_render_program_state_t program_state);

ui_runtime_render_program_t ui_runtime_render_program_state_program(ui_runtime_render_program_state_t program_state);

uint8_t ui_runtime_render_program_state_compatible(
    ui_runtime_render_program_state_t l, ui_runtime_render_program_state_t r, uint32_t flag);
    
void ui_runtime_render_program_state_attrs(
    ui_runtime_render_program_state_t program_state, ui_runtime_render_program_state_attr_it_t attr_it);

void ui_runtime_render_program_state_unifs(
    ui_runtime_render_program_state_t program_state, ui_runtime_render_program_state_unif_it_t unif_it);

int ui_runtime_render_program_state_set_attrs_by_buf_type(
    ui_runtime_render_program_state_t program_state, ui_runtime_render_buff_type_t buff_type);
    
int ui_runtime_render_program_state_set_unifs_buildin(
    ui_runtime_render_program_state_t program_state, ui_transform_t mv);
    
int ui_runtime_render_program_state_set_unif_f(
    ui_runtime_render_program_state_t program_state,
    const char * unif_name, float f);

int ui_runtime_render_program_state_set_unif_i(
    ui_runtime_render_program_state_t program_state,
    const char * unif_name, int32_t i);

int ui_runtime_render_program_state_set_unif_v2(
    ui_runtime_render_program_state_t program_state,
    const char * unif_name, ui_vector_2_t v2);

int ui_runtime_render_program_state_set_unif_v3(
    ui_runtime_render_program_state_t program_state,
    const char * unif_name, ui_vector_3_t v3);

int ui_runtime_render_program_state_set_unif_v4(
    ui_runtime_render_program_state_t program_state,
    const char * unif_name, ui_vector_4_t v4);
    
int ui_runtime_render_program_state_set_unif_m16(
    ui_runtime_render_program_state_t program_state,
    const char * unif_name, ui_transform_t m16);
    
int ui_runtime_render_program_state_set_unif_texture(
    ui_runtime_render_program_state_t program_state,
    const char * unif_name, 
    ui_cache_res_t res,
    ui_runtime_render_texture_filter_t min_filter,
    ui_runtime_render_texture_filter_t mag_filter,
    ui_runtime_render_texture_wrapping_t wrap_s,
    ui_runtime_render_texture_wrapping_t wrap_t,
    uint8_t texture_idx);

int ui_runtime_render_program_state_set_unif_texture_dft(
    ui_runtime_render_program_state_t program_state,
    ui_cache_res_t res,
    ui_runtime_render_texture_filter_t min_filter,
    ui_runtime_render_texture_filter_t mag_filter,
    ui_runtime_render_texture_wrapping_t wrap_s,
    ui_runtime_render_texture_wrapping_t wrap_t,
    uint8_t texture_idx);

ui_runtime_render_program_state_unif_data_t
ui_runtime_render_program_state_find_unif_texture_dft(
    ui_runtime_render_program_state_t program_state, uint8_t texture_idx);
    
#ifdef __cplusplus
}
#endif

#endif
