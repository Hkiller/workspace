#ifndef UI_RUNTIME_RENDER_H
#define UI_RUNTIME_RENDER_H
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif
    
ui_runtime_render_t
ui_runtime_render_create(ui_runtime_module_t module, uint32_t buf_capacity);
void ui_runtime_render_free(ui_runtime_render_t render);

ui_runtime_render_t ui_runtime_module_render(ui_runtime_module_t module);
    
ui_runtime_module_t ui_runtime_render_module(ui_runtime_render_t render);
gd_app_context_t ui_runtime_render_app(ui_runtime_render_t render);
ui_cache_manager_t ui_runtime_render_cache_mgr(ui_runtime_render_t render);
ui_data_mgr_t ui_runtime_render_data_mgr(ui_runtime_render_t render);

ui_color_t ui_runtime_render_clear_color(ui_runtime_render_t render);
void ui_runtime_render_set_clear_color(ui_runtime_render_t render, ui_color_t color);

ui_runtime_render_state_t ui_runtime_render_render_state(ui_runtime_render_t render);
    
ui_runtime_render_worker_t ui_runtime_render_worker(ui_runtime_render_t render);

uint32_t ui_runtime_render_data_buf_capacity(ui_runtime_render_t render);

ui_runtime_render_statistics_t ui_runtime_render_last_commit_statistics(ui_runtime_render_t render);

uint8_t ui_runtime_render_is_depth_test_for_2d(ui_runtime_render_t render);
void ui_runtime_render_set_is_depth_test_for_2d(ui_runtime_render_t render, uint8_t b);

/*render */    
int ui_runtime_render_init(ui_runtime_render_t render);
void ui_runtime_render_fini(ui_runtime_render_t render, uint8_t is_external_unloaded);    
    
/*matrix*/
int ui_runtime_render_matrix_push(ui_runtime_render_t render, ui_runtime_render_matrix_stack_type_t type);
void ui_runtime_render_matrix_pop(ui_runtime_render_t render, ui_runtime_render_matrix_stack_type_t type);
void ui_runtime_render_matrix_load_identity(ui_runtime_render_t render, ui_runtime_render_matrix_stack_type_t type);
void ui_runtime_render_matrix_load(ui_runtime_render_t render, ui_runtime_render_matrix_stack_type_t type, ui_transform_t mat);
void ui_runtime_render_matrix_multiply(ui_runtime_render_t render, ui_runtime_render_matrix_stack_type_t type, ui_transform_t mat);
ui_transform_t ui_runtime_render_matrix(ui_runtime_render_t render, ui_runtime_render_matrix_stack_type_t type);
int ui_runtime_render_matrix_reset(ui_runtime_render_t render);

/*scissor*/
int ui_runtime_render_scissor_push(ui_runtime_render_t render, ui_rect_t scissor);
void ui_runtime_render_scissor_pop(ui_runtime_render_t render);

/*view*/
ui_vector_2_t ui_runtime_render_view_size(ui_runtime_render_t render);
void ui_runtime_render_set_view_size(ui_runtime_render_t render, ui_vector_2_t view_size);

/*camera*/
ui_runtime_render_camera_t ui_runtime_render_active_camera(ui_runtime_render_t render);
void ui_runtime_render_set_active_camera(ui_runtime_render_t render, ui_runtime_render_camera_t camera);
    
/*commit*/    
void ui_runtime_render_begin(ui_runtime_render_t render, uint8_t * have_pre_frame);
void ui_runtime_render_done(ui_runtime_render_t render);

/*buildin*/
int ui_runtime_render_set_buildin_program_state(
    ui_runtime_render_t render,
    ui_runtime_render_program_buildin_t buildin,
    ui_runtime_render_program_state_t program_state);
    
#ifdef __cplusplus
}
#endif

#endif
