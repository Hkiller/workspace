#ifndef UI_UTILS_TYPES_H
#define UI_UTILS_TYPES_H
#include "cpe/pal/pal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* #define ui_pixel_aligned(x)	( (float)(int)(( x ) + 0.5f) ) */
#define ui_pixel_aligned(x)	((float)(int)x)
#define UI_FLOAT_PRECISION (0.001)
    
typedef struct ui_string_table * ui_string_table_t;
typedef struct ui_string_table_builder * ui_string_table_builder_t;
    
typedef struct ui_color ui_color, * ui_color_t;
typedef struct ui_rect ui_rect, * ui_rect_t;
typedef struct ui_vector_2 ui_vector_2, * ui_vector_2_t;
typedef struct ui_vector_3 ui_vector_3, * ui_vector_3_t;
typedef struct ui_vector_4 ui_vector_4, * ui_vector_4_t;
typedef struct ui_matrix_3x3 ui_matrix_3x3, * ui_matrix_3x3_t;
typedef struct ui_matrix_4x4 ui_matrix_4x4, * ui_matrix_4x4_t;
typedef struct ui_quaternion ui_quaternion, * ui_quaternion_t;
typedef struct ui_transform ui_transform, * ui_transform_t;

#ifdef __cplusplus
}
#endif

#endif
