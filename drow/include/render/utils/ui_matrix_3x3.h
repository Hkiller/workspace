#ifndef UI_UTILS_MATRIX_3X3_H
#define UI_UTILS_MATRIX_3X3_H
#include "cpe/utils/utils_types.h"
#include "ui_utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_matrix_3x3 {
	union {
		float		m[9];

		struct {
			float	element[3][3];
		};

		struct {
			float	m11, m21, m31;
			float	m12, m22, m32;
			float	m13, m23, m33;
		};
	};
};

int ui_matrix_3x3_cmp(ui_matrix_3x3_t l, ui_matrix_3x3_t r);

void ui_matrix_3x3_inline_mul_float(ui_matrix_3x3_t to, float f);
void ui_matrix_3x3_inline_div_float(ui_matrix_3x3_t div_to, float f);

void ui_matrix_3x3_inline_cross_product(ui_matrix_3x3_t to, ui_matrix_3x3_t o);
void ui_matrix_3x3_cross_product(ui_matrix_3x3_t to, ui_matrix_3x3_t l, ui_matrix_3x3_t r);
    
void ui_matrix_3x3_set_rotation_x_angle(ui_matrix_3x3_t m, float angle);
void ui_matrix_3x3_set_rotation_x_radians(ui_matrix_3x3_t m, float radians);

void ui_matrix_3x3_set_rotation_y_angle(ui_matrix_3x3_t m, float angle);
void ui_matrix_3x3_set_rotation_y_radians(ui_matrix_3x3_t m, float radians);

void ui_matrix_3x3_set_rotation_z_angle(ui_matrix_3x3_t m, float angle);
void ui_matrix_3x3_set_rotation_z_radians(ui_matrix_3x3_t m, float radians);

void ui_matrix_3x3_print(write_stream_t s, ui_matrix_3x3_t m, const char * prefix);

#define UI_INIT_IDENTITY_MATRIX_3X3 \
    { { { 1.0f, 0.0f, 0.0f,    \
          0.0f, 1.0f, 0.0f,     \
          0.0f, 0.0f, 1.0f } } }    \

#define UI_INIT_ZERO_MATRIX_3X3 \
    { { { 0.0f, 0.0f, 0.0f,    \
          0.0f, 0.0f, 0.0f,     \
          0.0f, 0.0f, 0.0f } } }    \

extern ui_matrix_3x3 UI_MATRIX_3X3_IDENTITY;
extern ui_matrix_3x3 UI_MATRIX_3X3_ZERO;
    
#ifdef __cplusplus
}
#endif

#endif
