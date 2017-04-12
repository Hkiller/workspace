#ifndef UI_RUNTIME_RENDER_BACKEND_H
#define UI_RUNTIME_RENDER_BACKEND_H
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif
        
/*render*/
typedef int (*ui_runtime_render_bind_fun_t)(void * ctx, ui_runtime_render_t render);
typedef void (*ui_runtime_render_unbind_fun_t)(void * ctx, ui_runtime_render_t render);

/*camera*/
typedef void (*ui_runtime_render_camera_update_fun_t)(void * ctx, ui_transform_t transform, ui_vector_2_t view_size, ui_runtime_render_camera_t camera);

/*program*/
typedef int (*ui_runtime_render_program_init_fun_t)(void * ctx, ui_runtime_render_program_t program);
typedef void (*ui_runtime_render_program_fini_fun_t)(void * ctx, ui_runtime_render_program_t program, uint8_t is_external_unloaded);
typedef int (*ui_runtime_render_program_attr_init_fun_t)(void * ctx, ui_runtime_render_program_attr_t program_attr);
typedef void (*ui_runtime_render_program_attr_fini_fun_t)(void * ctx, ui_runtime_render_program_attr_t program_attr);
typedef int (*ui_runtime_render_program_unif_init_fun_t)(void * ctx, ui_runtime_render_program_unif_t program_unif);
typedef void (*ui_runtime_render_program_unif_fini_fun_t)(void * ctx, ui_runtime_render_program_unif_t program_unif);

/*env*/    
typedef void (*ui_runtime_render_state_save_fun_t)(void * ctx, ui_runtime_render_state_data_t state_data);
typedef void (*ui_runtime_render_state_restore_fun_t)(void * ctx, ui_runtime_render_state_data_t state_data);
    
/*commit*/
typedef void (*ui_runtime_render_commit_begin_fun_t)(void * ctx, ui_runtime_render_t render);
typedef void (*ui_runtime_render_commit_done_fun_t)(void * ctx, ui_runtime_render_t render, ui_runtime_render_statistics_t statistics);
typedef void (*ui_runtime_render_commit_group_begin_fun_t)(void * ctx, ui_runtime_render_t render, ui_runtime_render_queue_group_t cmd_group_type);
typedef void (*ui_runtime_render_commit_group_done_fun_t)(void * ctx, ui_runtime_render_t render, ui_runtime_render_queue_group_t cmd_group_type);
typedef void (*ui_runtime_render_commit_cmd_fun_t)(void * ctx, ui_runtime_render_t render, ui_runtime_render_cmd_t cmd);
    
ui_runtime_render_backend_t
ui_runtime_render_backend_create(
    ui_runtime_module_t module, const char * name,
    void * ctx,
    /*render*/
    ui_runtime_render_bind_fun_t render_bind,
    ui_runtime_render_unbind_fun_t render_unbind,
    /*camera*/
    ui_runtime_render_camera_update_fun_t camera_update,
    /*program*/
    uint32_t program_capacity,
    ui_runtime_render_program_init_fun_t program_init,
    ui_runtime_render_program_fini_fun_t program_fini,
    /*program attr*/
    uint32_t program_attr_capacity,
    ui_runtime_render_program_attr_init_fun_t program_attr_init,
    ui_runtime_render_program_attr_fini_fun_t program_attr_fini,
    /*program unif*/
    uint32_t program_unif_capacity,
    ui_runtime_render_program_unif_init_fun_t program_unif_init,
    ui_runtime_render_program_unif_fini_fun_t program_unif_fini,
    /*env*/
    ui_runtime_render_state_save_fun_t state_save,
    ui_runtime_render_state_restore_fun_t state_restore,
    /*commit*/
    ui_runtime_render_commit_begin_fun_t commit_being,
    ui_runtime_render_commit_done_fun_t commit_done,
    ui_runtime_render_commit_group_begin_fun_t commit_group_being,
    ui_runtime_render_commit_group_done_fun_t commit_group_done,
    ui_runtime_render_commit_cmd_fun_t commit_cmd);

ui_runtime_render_backend_t
ui_runtime_render_backend_find_by_name(
    ui_runtime_module_t module, const char * name);
    
void ui_runtime_render_backend_free(ui_runtime_render_backend_t backend);

#ifdef __cplusplus
}
#endif

#endif 
