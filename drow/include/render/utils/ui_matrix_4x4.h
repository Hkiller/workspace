#ifndef UI_UTILS_MATRIX_4X4_H
#define UI_UTILS_MATRIX_4X4_H
#include "cpe/utils/utils_types.h"
#include "ui_utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_matrix_4x4 {
	union {
		float		m[16];

		struct {
			float	m11, m21, m31, m41;
			float	m12, m22, m32, m42;
			float	m13, m23, m33, m43;
			float	m14, m24, m34, m44;
		};

		struct {
			float	element[4][4];
		};
	};
};

int ui_matrix_4x4_cmp(ui_matrix_4x4_t l, ui_matrix_4x4_t r);

void ui_matrix_4x4_inline_add(ui_matrix_4x4_t add_to, ui_matrix_4x4_t o);
void ui_matrix_4x4_inline_sub(ui_matrix_4x4_t sub_to, ui_matrix_4x4_t o);

void ui_matrix_4x4_inline_cross_product(ui_matrix_4x4_t to, ui_matrix_4x4_t o);

void ui_matrix_4x4_inline_mul_float(ui_matrix_4x4_t to, float f);

void ui_matrix_4x4_inline_div_float(ui_matrix_4x4_t div_to, float f);

void ui_matrix_4x4_cross_product(ui_matrix_4x4_t to, ui_matrix_4x4_t l, ui_matrix_4x4_t r);

uint8_t ui_matrix_4x4_is_identity(ui_matrix_4x4_t m);
void ui_matrix_4x4_set_identity(ui_matrix_4x4_t m);

void ui_matrix_4x4_set_scale(ui_matrix_4x4_t m, float x, float y, float z);
void ui_matrix_4x4_set_scale_vector_3(ui_matrix_4x4_t m, ui_vector_3_t v);
void ui_matrix_4x4_set_scale_vector_2(ui_matrix_4x4_t m, ui_vector_2_t v);
    
void ui_matrix_4x4_set_translation(ui_matrix_4x4_t m, float x, float y, float z);

/*adj*/    
void ui_matrix_4x4_adj_vector_2(ui_matrix_4x4_t m, ui_vector_2_t to, ui_vector_2_t i);
void ui_matrix_4x4_inline_adj_vector_2(ui_matrix_4x4_t m, ui_vector_2_t v);

void ui_matrix_4x4_adj_vector_3(ui_matrix_4x4_t m, ui_vector_3_t to, ui_vector_3_t i);
void ui_matrix_4x4_inline_adj_vector_3(ui_matrix_4x4_t m, ui_vector_3_t v);

/*determinant*/
float ui_matrix_4x4_determinant(ui_matrix_4x4_t m);
    
/*invers*/
void ui_matrix_4x4_invers(ui_matrix_4x4_t to, ui_matrix_4x4_t origin);
void ui_matrix_4x4_inline_invers(ui_matrix_4x4_t to);

/*print*/    
void ui_matrix_4x4_print(write_stream_t s, ui_matrix_4x4_t m, const char * prefix);

/*init*/
#define UI_MATRIX_4X4_INITLIZER(                \
    __m11, __m21, __m31, __m41,                 \
    __m12, __m22, __m32, __m42,                 \
    __m13, __m23, __m33, __m43,                 \
    __m14, __m24, __m34, __m44)                 \
    { { {                                       \
        __m11, __m21, __m31, __m41,             \
        __m12, __m22, __m32, __m42,             \
        __m13, __m23, __m33, __m43,             \
        __m14, __m24, __m34, __m44              \
    } } }
    
#define UI_INIT_IDENTITY_MATRIX_4X4 \
    { { { 1.0f, 0.0f, 0.0f, 0.0f,    \
          0.0f, 1.0f, 0.0f, 0.0f,     \
          0.0f, 0.0f, 1.0f, 0.0f,     \
          0.0f, 0.0f, 0.0f, 1.0f } } }    \

#define UI_INIT_ZERO_MATRIX_4X4 \
    { { { 0.0f, 0.0f, 0.0f, 0.0f,    \
          0.0f, 0.0f, 0.0f, 0.0f,     \
          0.0f, 0.0f, 0.0f, 0.0f,     \
          0.0f, 0.0f, 0.0f, 0.0f } } }    \

extern ui_matrix_4x4 UI_MATRIX_4X4_IDENTITY;
extern ui_matrix_4x4 UI_MATRIX_4X4_ZERO;
    
#ifdef __cplusplus
}
#endif

#endif
