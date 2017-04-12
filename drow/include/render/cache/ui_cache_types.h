#ifndef UI_CACHE_TYPES_H
#define UI_CACHE_TYPES_H
#include "cpe/pal/pal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ui_cache_res_type {
    ui_cache_res_type_texture = 1,
    ui_cache_res_type_sound = 2,
    ui_cache_res_type_font = 3,
} ui_cache_res_type_t;

typedef enum ui_cache_pixel_format {
    ui_cache_pf_unknown = 0,
    ui_cache_pf_pal8,
    ui_cache_pf_pala8,
    ui_cache_pf_r5g6b5,
    ui_cache_pf_r4g4b4a4,
    ui_cache_pf_r5g5b5a1,
    ui_cache_pf_r8g8b8,
    ui_cache_pf_r8g8b8a8,
    ui_cache_pf_a8,
    ui_cache_pf_s8,
    ui_cache_pf_d16,
    ui_cache_pf_dxt1,
    ui_cache_pf_dxt3,
    ui_cache_pf_dxt5,
    ui_cache_pf_rgbpvrtc2,
    ui_cache_pf_rgbapvrtc2,
    ui_cache_pf_rgbpvrtc4,
    ui_cache_pf_rgbapvrtc4,
    ui_cache_pf_format_count,
} ui_cache_pixel_format_t;

typedef enum ui_cache_res_load_state {
    ui_cache_res_not_load = 0,
    ui_cache_res_wait_load = 1,    
    ui_cache_res_loading = 2,
    ui_cache_res_data_loaded = 3,
    ui_cache_res_loaded = 4,
    ui_cache_res_load_fail = 5,
    ui_cache_res_cancel_loading = 6,
} ui_cache_res_load_state_t;

typedef enum ui_cache_res_load_result {
    ui_cache_res_load_success = 0,
    ui_cache_res_internal_error,
    ui_cache_res_file_not_exist,
} ui_cache_res_load_result_t;

typedef enum ui_cache_pixel_rect_flip_type {
    ui_cache_pixel_rect_flip_type_none = 0
    , ui_cache_pixel_rect_flip_type_x = 1
    , ui_cache_pixel_rect_flip_type_y = 2
    , ui_cache_pixel_rect_flip_type_xy = 3
} ui_cache_pixel_rect_flip_type_t;

typedef enum ui_cache_pixel_field {
    ui_cache_pixel_field_r = 0
    , ui_cache_pixel_field_g = 1
    , ui_cache_pixel_field_b = 2
    , ui_cache_pixel_field_a = 3
    , ui_cache_pixel_field_argb = 4
} ui_cache_pixel_field_t;

typedef enum ui_cache_res_using_state {
    ui_cache_res_using_state_free = 0,
    ui_cache_res_using_state_week = 1,
    ui_cache_res_using_state_ref_count = 2,
    ui_cache_res_using_state_lock = 3,
} ui_cache_res_using_state_t;
    
typedef enum ui_cache_pixel_rect_angle_type {
    ui_cache_pixel_rect_angle_type_none = 0,
    ui_cache_pixel_rect_angle_type_90 = 1,
    ui_cache_pixel_rect_angle_type_180 = 2,
    ui_cache_pixel_rect_angle_type_270 = 3,
} ui_cache_pixel_rect_angle_type_t;

typedef enum ui_cache_sound_format {
    ui_cache_sound_format_unknown = 0,
    ui_cache_sound_format_ogg = 1,
} ui_cache_sound_format_t;

typedef enum ui_cache_sound_data_format {
    ui_cache_sound_data_format_mono8 = 1,
    ui_cache_sound_data_format_mono16 = 2,
    ui_cache_sound_data_format_stereo8 = 3,
    ui_cache_sound_data_format_stereo16 = 4,
} ui_cache_sound_data_format_t;
    
typedef struct ui_cache_group * ui_cache_group_t;
typedef struct ui_cache_res * ui_cache_res_t;
typedef struct ui_cache_res_it * ui_cache_res_it_t;
typedef struct ui_cache_res_ref * ui_cache_res_ref_t;
typedef struct ui_cache_manager * ui_cache_manager_t;
typedef struct ui_cache_worker * ui_cache_worker_t;
typedef struct ui_cache_res_plugin * ui_cache_res_plugin_t;
typedef struct ui_cache_evt_collector * ui_cache_evt_collector_t;

/*texture*/
typedef struct ui_cache_pixel_level_info * ui_cache_pixel_level_info_t;
typedef struct ui_cache_pixel_buf * ui_cache_pixel_buf_t;
typedef struct ui_cache_pixel_buf_rect * ui_cache_pixel_buf_rect_t;

/*sound*/
typedef struct ui_cache_sound_buf * ui_cache_sound_buf_t;
    
#ifdef __cplusplus
}
#endif

#endif
