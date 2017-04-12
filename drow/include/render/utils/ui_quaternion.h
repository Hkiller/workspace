#ifndef UI_UTILS_QUATERNION_H
#define UI_UTILS_QUATERNION_H
#include "cpe/utils/utils_types.h"
#include "ui_utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_quaternion {
	union {
		float value[4];
		struct {
			float w;
			float x;
			float y;
			float z;
		};
	};
};

int ui_quaternion_cmp(ui_quaternion_t l, ui_quaternion_t r);
    
void ui_quaternion_to_rotation_matrix_4x4(ui_quaternion_t quaternion, ui_matrix_4x4_t m);
void ui_quaternion_to_rotation_matrix_3x3(ui_quaternion_t quaternion, ui_matrix_3x3_t m);    
void ui_quaternion_set_x_radians(ui_quaternion_t quaternion, float radians);
void ui_quaternion_set_y_radians(ui_quaternion_t quaternion, float radians);
void ui_quaternion_set_z_radians(ui_quaternion_t quaternion, float radians);
void ui_quaternion_set_x_angle(ui_quaternion_t quaternion, float angle);
void ui_quaternion_set_y_angle(ui_quaternion_t quaternion, float angle);
void ui_quaternion_set_z_angle(ui_quaternion_t quaternion, float angle);

void ui_quaternion_set_radians(ui_quaternion_t quaternion, ui_vector_3_t radians);
void ui_quaternion_get_radians(ui_quaternion_t quaternion, ui_vector_3_t radians);
    
float ui_quaternion_length(ui_quaternion_t quaternion);
float ui_quaternion_length_square(ui_quaternion_t quaternion);

void ui_quaternion_get_normalize(ui_quaternion_t quaternion);
    
void ui_quaternion_cross_product(ui_quaternion_t to, ui_quaternion_t l, ui_quaternion_t r);
void ui_quaternion_inline_cross_product(ui_quaternion_t to, ui_quaternion_t o);

void ui_quaternion_adj_vector_2(ui_quaternion_t q, ui_vector_2_t to, ui_vector_2_t i);
void ui_quaternion_inline_adj_vector_2(ui_quaternion_t q, ui_vector_2_t v);
    
void ui_quaternion_adj_vector_3(ui_quaternion_t q, ui_vector_3_t to, ui_vector_3_t i);
void ui_quaternion_inline_adj_vector_3(ui_quaternion_t q, ui_vector_3_t v);

void ui_quaternion_print(write_stream_t s, ui_quaternion_t t, const char * prefix);

#define UI_QUATERNION_INITLIZER(__w, __x, __y, __z) { { { (__w), (__x), (__y), (__z) } } }

extern ui_quaternion UI_QUATERNION_IDENTITY;
extern ui_quaternion UI_QUATERNION_ZERO;
    
#ifdef __cplusplus
}
#endif

#endif
