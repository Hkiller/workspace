#ifndef DROW_PLUGIN_LAYOUT_TYPES_H
#define DROW_PLUGIN_LAYOUT_TYPES_H
#include "render/utils/ui_utils_types.h"
#include "render/runtime/ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_layout_module * plugin_layout_module_t;
typedef struct plugin_layout_render * plugin_layout_render_t;
typedef struct plugin_layout_render_node * plugin_layout_render_node_t;
typedef struct plugin_layout_render_node_it * plugin_layout_render_node_it_t;
typedef struct plugin_layout_render_group * plugin_layout_render_group_t;
typedef struct plugin_layout_layout * plugin_layout_layout_t;
typedef struct plugin_layout_layout_meta * plugin_layout_layout_meta_t;
typedef struct plugin_layout_layout_basic * plugin_layout_layout_basic_t;
typedef struct plugin_layout_layout_rich * plugin_layout_layout_rich_t;
typedef struct plugin_layout_layout_rich_block * plugin_layout_layout_rich_block_t;
typedef struct plugin_layout_font_id * plugin_layout_font_id_t;
typedef struct plugin_layout_font_draw * plugin_layout_font_draw_t;
typedef struct plugin_layout_font_cache * plugin_layout_font_cache_t;    
typedef struct plugin_layout_font_meta * plugin_layout_font_meta_t;
typedef struct plugin_layout_font_face * plugin_layout_font_face_t;
typedef struct plugin_layout_font_element * plugin_layout_font_element_t;
typedef struct plugin_layout_animation_meta * plugin_layout_animation_meta_t;
typedef struct plugin_layout_animation * plugin_layout_animation_t;
typedef struct plugin_layout_animation_caret * plugin_layout_animation_caret_t;
typedef struct plugin_layout_animation_selection * plugin_layout_animation_selection_t;

typedef struct plugin_layout_font_meta_font * plugin_layout_font_meta_font_t;
typedef struct plugin_layout_font_face_font * plugin_layout_font_face_font_t;
typedef struct plugin_layout_font_element_font * plugin_layout_font_element_font_t;

typedef struct plugin_layout_font_face_pic * plugin_layout_font_face_pic_t;
    
typedef enum {
    plugin_layout_align_left_top,
    plugin_layout_align_center_top,
    plugin_layout_align_right_top,
    plugin_layout_align_left_center,
    plugin_layout_align_center_center,
    plugin_layout_align_right_center,
    plugin_layout_align_left_bottom,
    plugin_layout_align_center_bottom,
    plugin_layout_align_right_bottom,
} plugin_layout_align_t;
    
typedef enum plugin_layout_font_category {
    plugin_layout_font_category_font = 1,
    plugin_layout_font_category_pic = 2,
} plugin_layout_font_category_t;

typedef enum plugin_layout_font_draw_flag {
    plugin_layout_font_draw_flag_shadow = 1,
    plugin_layout_font_draw_flag_undline = 2,
} plugin_layout_font_draw_flag_t;

typedef enum plugin_layout_animation_layer {
    plugin_layout_animation_layer_before = 1,
    plugin_layout_animation_layer_after = 2,
} plugin_layout_animation_layer_t;

#ifdef __cplusplus
}
#endif

#endif
