#ifndef UI_RUNTIME_TYPES_H
#define UI_RUNTIME_TYPES_H
#include "render/utils/ui_utils_types.h"
#include "render/cache/ui_cache_types.h"
#include "render/model/ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_runtime_module * ui_runtime_module_t;
typedef struct ui_runtime_render * ui_runtime_render_t;
typedef struct ui_runtime_render_backend * ui_runtime_render_backend_t;
typedef struct ui_runtime_render_queue * ui_runtime_render_queue_t;
typedef struct ui_runtime_render_cmd * ui_runtime_render_cmd_t;
typedef struct ui_runtime_render_camera * ui_runtime_render_camera_t;
typedef struct ui_runtime_render_vertex_buff * ui_runtime_render_vertex_buff_t;
typedef struct ui_runtime_render_buff_use * ui_runtime_render_buff_use_t;
typedef struct ui_runtime_render_worker * ui_runtime_render_worker_t;
typedef struct ui_runtime_render_obj_meta * ui_runtime_render_obj_meta_t;
typedef struct ui_runtime_render_obj_meta_it * ui_runtime_render_obj_meta_it_t;
typedef struct ui_runtime_render_obj * ui_runtime_render_obj_t;
typedef struct ui_runtime_render_obj_ref * ui_runtime_render_obj_ref_t;
typedef struct ui_runtime_render_second_color * ui_runtime_render_second_color_t;

typedef struct ui_runtime_render_shader * ui_runtime_render_shader_t;
typedef struct ui_runtime_render_program * ui_runtime_render_program_t;
typedef struct ui_runtime_render_program_attr * ui_runtime_render_program_attr_t;
typedef struct ui_runtime_render_program_attr_it * ui_runtime_render_program_attr_it_t;
typedef struct ui_runtime_render_program_unif * ui_runtime_render_program_unif_t;
typedef struct ui_runtime_render_program_unif_it * ui_runtime_render_program_unif_it_t;
    
typedef struct ui_runtime_render_program_state * ui_runtime_render_program_state_t;
typedef struct ui_runtime_render_program_state_attr * ui_runtime_render_program_state_attr_t;
typedef struct ui_runtime_render_program_state_attr_it * ui_runtime_render_program_state_attr_it_t;
typedef struct ui_runtime_render_program_state_attr_data * ui_runtime_render_program_state_attr_data_t;
typedef struct ui_runtime_render_program_state_unif * ui_runtime_render_program_state_unif_t;
typedef struct ui_runtime_render_program_state_unif_it * ui_runtime_render_program_state_unif_it_t;
typedef struct ui_runtime_render_program_state_unif_data * ui_runtime_render_program_state_unif_data_t;
typedef struct ui_runtime_render_state * ui_runtime_render_state_t;
typedef struct ui_runtime_render_state_data * ui_runtime_render_state_data_t;

typedef struct ui_runtime_render_material * ui_runtime_render_material_t;    
typedef struct ui_runtime_render_technique * ui_runtime_render_technique_t;
typedef struct ui_runtime_render_technique_it * ui_runtime_render_technique_it_t;
typedef struct ui_runtime_render_pass * ui_runtime_render_pass_t;
typedef struct ui_runtime_render_pass_it * ui_runtime_render_pass_it_t;

/*runing*/
typedef enum ui_runtime_runing_level {
    ui_runtime_stop, /*清理占用资源 */
    ui_runtime_pause, /*暂停 */
    ui_runtime_runing,
} ui_runtime_runing_level_t;

/*sound*/
typedef enum ui_runtime_sound_type {
    ui_runtime_sound_sfx,
    ui_runtime_sound_bgm,
} ui_runtime_sound_type_t;
    
typedef enum ui_runtime_sound_group_schedule_type {
    ui_runtime_sound_group_schedule_none,
    ui_runtime_sound_group_schedule_queue,
} ui_runtime_sound_group_schedule_type_t;

typedef enum ui_runtime_sound_chanel_state {
    ui_runtime_sound_chanel_initial,
    ui_runtime_sound_chanel_playing,
    ui_runtime_sound_chanel_paused,
    ui_runtime_sound_chanel_stopped,
} ui_runtime_sound_chanel_state_t;
    
typedef struct ui_runtime_sound_backend * ui_runtime_sound_backend_t;
typedef struct ui_runtime_sound_res * ui_runtime_sound_res_t;    
typedef struct ui_runtime_sound_chanel * ui_runtime_sound_chanel_t;
typedef struct ui_runtime_sound_group * ui_runtime_sound_group_t;
typedef struct ui_runtime_sound_playing * ui_runtime_sound_playing_t;    

/*render */
typedef struct ui_runtime_render_statistics {
    uint32_t m_cmd_count;
    uint32_t m_draw_call_count;
    uint32_t m_triangles_count;
    uint32_t m_vertex_count;
} * ui_runtime_render_statistics_t;
    
typedef enum ui_runtime_render_matrix_stack_type {
    /* Model view matrix stack */
    ui_runtime_render_matrix_stack_modelview,
    /* projection matrix stack */
    ui_runtime_render_matrix_stack_projection,
    /* texture matrix stack */
    ui_runtime_render_matrix_stack_texture,
} ui_runtime_render_matrix_stack_type_t;

typedef enum ui_runtime_render_projection {
    /* Sets a 2D projection (orthogonal projection). */
    ui_runtime_render_projection_2d,
    /* Sets a 3D projection with a fovy=60, znear=0.5f and zfar=1500. */
    ui_runtime_render_projection_3d,
    /* It calls "updateProjection" on the projection delegate. */
    ui_runtime_render_projection_custom,
} ui_runtime_render_projection_t;

typedef enum ui_runtime_render_blend_factor {
    ui_runtime_render_dst_alpha,
    ui_runtime_render_dst_color,
    ui_runtime_render_one,
    ui_runtime_render_one_minus_dst_alpha,
    ui_runtime_render_one_minus_dst_color,
    ui_runtime_render_one_minus_src_alpha,
    ui_runtime_render_one_minus_src_color,
    ui_runtime_render_src_alpha,
    ui_runtime_render_src_color,
    ui_runtime_render_zero,
} ui_runtime_render_blend_factor_t;
    
typedef struct ui_runtime_render_blend {
    ui_runtime_render_blend_factor_t m_src_factor;
    ui_runtime_render_blend_factor_t m_dst_factor;
} * ui_runtime_render_blend_t;

typedef enum ui_runtime_render_depth_function {    
    ui_runtime_render_depth_never,
    ui_runtime_render_depth_less,
    ui_runtime_render_depth_equal,
    ui_runtime_render_depth_lequal,
    ui_runtime_render_depth_greater,
    ui_runtime_render_depth_notequal,
    ui_runtime_render_depth_gequal,
    ui_runtime_render_depth_always,
} ui_runtime_render_depth_function_t;

typedef enum ui_runtime_render_cull_face {    
    ui_runtime_render_cull_face_none,
    ui_runtime_render_cull_face_back,
    ui_runtime_render_cull_face_front,
    ui_runtime_render_cull_face_front_and_back,
} ui_runtime_render_cull_face_t;

typedef enum ui_runtime_render_front_face {
    ui_runtime_render_front_face_cw,
    ui_runtime_render_front_face_ccw,
} ui_runtime_render_front_face_t;

typedef enum ui_runtime_render_stencil_function {
    ui_runtime_render_stencil_never,
    ui_runtime_render_stencil_always,
    ui_runtime_render_stencil_less,
    ui_runtime_render_stencil_lequal,
    ui_runtime_render_stencil_equal,
    ui_runtime_render_stencil_greater,
    ui_runtime_render_stencil_gequal,
    ui_runtime_render_stencil_notequal,
} ui_runtime_render_stencil_function_t;

typedef enum ui_runtime_render_stencil_op {
    ui_runtime_render_stencil_op_keep,
    ui_runtime_render_stencil_op_zero,
    ui_runtime_render_stencil_op_replace,
    ui_runtime_render_stencil_op_incr,
    ui_runtime_render_stencil_op_decr,
    ui_runtime_render_stencil_op_invert,
    ui_runtime_render_stencil_op_incr_wrap,
    ui_runtime_render_stencil_op_decr_wrap,
} ui_runtime_render_stencil_op_t;

typedef enum ui_runtime_render_filter {
    ui_runtime_render_filter_linear,
    ui_runtime_render_filter_nearest,
} ui_runtime_render_texture_filter_t;
    
typedef enum ui_runtime_render_texture_wrapping {
    ui_runtime_render_texture_repeat,
    ui_runtime_render_texture_mirrored_repeat,
    ui_runtime_render_texture_clamp_to_edge,
    ui_runtime_render_texture_clamp_to_border,
} ui_runtime_render_texture_wrapping_t;

typedef enum ui_runtime_render_queue_group {
    ui_runtime_render_queue_group_neg, /**Objects with globalZ smaller than 0.*/
    ui_runtime_render_queue_group_3d, /**Opaque 3D objects with 0 globalZ.*/
    ui_runtime_render_queue_group_3d_transparent = 2, /**Transparent 3D objects with 0 globalZ.*/
    ui_runtime_render_queue_group_zero = 3, /**2D objects with 0 globalZ.*/
    ui_runtime_render_queue_group_pos = 4, /**Objects with globalZ bigger than 0.*/
    ui_runtime_render_queue_group_count = 5,
} ui_runtime_render_queue_group_t;

typedef enum ui_runtime_render_program_attr_id {
    ui_runtime_render_program_attr_position,
    ui_runtime_render_program_attr_normal,
    ui_runtime_render_program_attr_binormal,
    ui_runtime_render_program_attr_tangent,

    ui_runtime_render_program_attr_texcoord,
    ui_runtime_render_program_attr_texcoord0 = ui_runtime_render_program_attr_texcoord,
    ui_runtime_render_program_attr_texcoord1,
    ui_runtime_render_program_attr_texcoord2,
    ui_runtime_render_program_attr_texcoord3,
    ui_runtime_render_program_attr_texcoord4,
    ui_runtime_render_program_attr_texcoord5,
    ui_runtime_render_program_attr_texcoord6,
    ui_runtime_render_program_attr_texcoord7,
    ui_runtime_render_program_attr_texcoord8,
    ui_runtime_render_program_attr_texcoord9,

    ui_runtime_render_program_attr_blendindices,
    ui_runtime_render_program_attr_blendweight,
    ui_runtime_render_program_attr_color,
    ui_runtime_render_program_attr_index,

    ui_runtime_render_program_attr_bonematrices,
    ui_runtime_render_program_attr_bonepalette,

    ui_runtime_render_program_attr_transforms,
    ui_runtime_render_program_attr_instancetransforms,

    ui_runtime_render_program_attr_user,

    ui_runtime_render_program_attr_max
} ui_runtime_render_program_attr_id_t;

typedef enum ui_runtime_render_program_buildin {
    ui_runtime_render_program_buildin_tex,
    ui_runtime_render_program_buildin_add,
    ui_runtime_render_program_buildin_multiply,
    ui_runtime_render_program_buildin_color,
    ui_runtime_render_program_buildin_count,
} ui_runtime_render_program_buildin_t;

typedef enum ui_runtime_render_program_unif_type {
    ui_runtime_render_program_unif_f,
    ui_runtime_render_program_unif_i,
    ui_runtime_render_program_unif_v2,
    ui_runtime_render_program_unif_v3,
    ui_runtime_render_program_unif_v4,
    ui_runtime_render_program_unif_m16,
    ui_runtime_render_program_unif_texture,
} ui_runtime_render_program_unif_type_t;

typedef enum ui_runtime_render_second_color_mix {
    ui_runtime_render_second_color_none,
    ui_runtime_render_second_color_add,
    ui_runtime_render_second_color_multiply,
    ui_runtime_render_second_color_color,
} ui_runtime_render_second_color_mix_t;

typedef enum ui_runtime_render_program_unif_buildin {
    ui_runtime_render_program_unif_matrix_p,
    ui_runtime_render_program_unif_matrix_mv,
    ui_runtime_render_program_unif_matrix_mvp,
    ui_runtime_render_program_unif_buildin_count,
} ui_runtime_render_program_unif_buildin_t;
    
typedef enum ui_runtime_render_buff_type {
    ui_runtime_render_buff_index_uint16,
    ui_runtime_render_buff_vertex_v3f_t2f_c4b,
} ui_runtime_render_buff_type_t;

typedef struct ui_runtime_vertex_v3f_t2f_c4ub ui_runtime_vertex_v3f_t2f_c4ub, * ui_runtime_vertex_v3f_t2f_c4ub_t;

typedef enum ui_runtime_render_env_compatible_flag {
    ui_runtime_render_render_env_compatible_ignore_mvp = 1u,
} ui_runtime_render_env_compatible_flag_t;
    
typedef void (*ui_runtime_render_obj_evt_fun_t)(void * ctx, ui_runtime_render_obj_t obj, const char * evt);

/*render obj touch */
typedef enum ui_runtime_touch_event {
    ui_runtime_touch_begin,
    ui_runtime_touch_move,
    ui_runtime_touch_end,
} ui_runtime_touch_event_t;
    
typedef void (*ui_runtime_touch_process_fun_t)(
    void * ctx, ui_runtime_render_obj_t obj,
    uint32_t track_id, ui_runtime_touch_event_t evt, ui_vector_2_t screen_pt, ui_vector_2_t logic_pt);
    
#ifdef __cplusplus
}
#endif

#endif
