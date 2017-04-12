#include "cpe/pal/pal_strings.h"
#include "cpe/utils/stream.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_matrix_3x3.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_vector_3.h"

int ui_matrix_3x3_cmp(ui_matrix_3x3_t l, ui_matrix_3x3_t r) {
    uint8_t i;
    int rv;

    for(i = 0; i < CPE_ARRAY_SIZE(l->m); ++i) {
        if ((rv = cpe_float_cmp(l->m[i], r->m[i], UI_FLOAT_PRECISION))) return rv;
    }
    
    return 0;
}

void ui_matrix_3x3_inline_add(ui_matrix_3x3_t add_to, ui_matrix_3x3_t o) {
	add_to->m11 += o->m11; add_to->m12 += o->m12; add_to->m13 += o->m13;
    add_to->m21 += o->m21; add_to->m22 += o->m22; add_to->m23 += o->m23;
    add_to->m31 += o->m31; add_to->m32 += o->m32; add_to->m33 += o->m33;
}

void ui_matrix_3x3_inline_sub(ui_matrix_3x3_t sub_to, ui_matrix_3x3_t o) {
	sub_to->m11 -= o->m11; sub_to->m12 -= o->m12; sub_to->m13 -= o->m13;
    sub_to->m21 -= o->m21; sub_to->m22 -= o->m22; sub_to->m23 -= o->m23;
    sub_to->m31 -= o->m31; sub_to->m32 -= o->m32; sub_to->m33 -= o->m33;
}

void ui_matrix_3x3_cross_product(ui_matrix_3x3_t to, ui_matrix_3x3_t l, ui_matrix_3x3_t r) {
    to->m11 = l->m11 * r->m11 + l->m12 * r->m21 + l->m13 * r->m31;
    to->m12 = l->m11 * r->m12 + l->m12 * r->m22 + l->m13 * r->m32;
    to->m13 = l->m11 * r->m13 + l->m12 * r->m23 + l->m13 * r->m33;

    to->m21 = l->m21 * r->m11 + l->m22 * r->m21 + l->m23 * r->m31;
    to->m22 = l->m21 * r->m12 + l->m22 * r->m22 + l->m23 * r->m32;
    to->m23 = l->m21 * r->m13 + l->m22 * r->m23 + l->m23 * r->m33;
    
    to->m31 = l->m31 * r->m11 + l->m32 * r->m21 + l->m33 * r->m31;
    to->m32 = l->m31 * r->m12 + l->m32 * r->m22 + l->m33 * r->m32;
    to->m33 = l->m31 * r->m13 + l->m32 * r->m23 + l->m33 * r->m33;
}

void ui_matrix_3x3_inline_cross_product(ui_matrix_3x3_t to, ui_matrix_3x3_t o) {
    ui_matrix_3x3 r;
    ui_matrix_3x3_cross_product(&r, to, o);
    *to = r;
}

void ui_matrix_3x3_inline_mul_float(ui_matrix_3x3_t to, float f) {
    to->m11 *= f;  to->m12 *= f;  to->m13 *= f;
    to->m21 *= f;  to->m22 *= f;  to->m23 *= f;
    to->m31 *= f;  to->m32 *= f;  to->m33 *= f;
}

void ui_matrix_3x3_inline_div_float(ui_matrix_3x3_t div_to, float f)  {
    div_to->m11 /= f;  div_to->m12 /= f;  div_to->m13 /= f;
    div_to->m21 /= f;  div_to->m22 /= f;  div_to->m23 /= f;
    div_to->m31 /= f;  div_to->m32 /= f;  div_to->m33 /= f;
}

void ui_matrix_3x3_print(write_stream_t s, ui_matrix_3x3_t m, const char * prefix) {
    uint8_t i;
    for(i = 0; i < 3; i++) {
        stream_printf(
            s, "%s[ %3.2f, %3.2f, %3.2f ]\n",
            prefix, m->element[0][i], m->element[1][i], m->element[2][i]);
    }
}

void ui_matrix_3x3_set_rotation_x_angle(ui_matrix_3x3_t m, float angle) {
	float fs = cpe_sin_angle(angle);
	float fc = cpe_cos_angle(angle);
    
	m->m11 = 1.0f; m->m12 = 0.0f; m->m13 = 0.0f;
	m->m21 = 0.0f; m->m22 = fc;	  m->m23 = -fs;
	m->m31 = 0.0f; m->m32 = fs;	  m->m33 =  fc;
}

void ui_matrix_3x3_set_rotation_x_radians(ui_matrix_3x3_t m, float radians) {
	float fs = cpe_sin_radians(radians);
	float fc = cpe_cos_radians(radians);
    
	m->m11 = 1.0f; m->m12 = 0.0f; m->m13 = 0.0f;
	m->m21 = 0.0f; m->m22 = fc;	  m->m23 = -fs;
	m->m31 = 0.0f; m->m32 = fs;	  m->m33 =  fc;
}    

void ui_matrix_3x3_set_rotation_y_angle(ui_matrix_3x3_t m, float angle) {
	float fs = cpe_sin_angle(angle);
	float fc = cpe_cos_angle(angle);
    
	m->m11 = fc;	m->m12 = 0.0f; m->m13 = fs;
	m->m21 = 0.0f;  m->m22 = 1.0f; m->m23 = 0.0f;
	m->m31 = -fs;	m->m32 = 0.0f; m->m33 = fc;
}

void ui_matrix_3x3_set_rotation_y_radians(ui_matrix_3x3_t m, float radians) {
	float fs = cpe_sin_radians(radians);
	float fc = cpe_cos_radians(radians);
    
	m->m11 = fc;	m->m12 = 0.0f; m->m13 = fs;
	m->m21 = 0.0f;  m->m22 = 1.0f; m->m23 = 0.0f;
	m->m31 = -fs;	m->m32 = 0.0f; m->m33 = fc;
}

void ui_matrix_3x3_set_rotation_z_angle(ui_matrix_3x3_t m, float angle) {
	float fs = cpe_sin_angle(angle);
	float fc = cpe_cos_angle(angle);

	m->m11 = fc;	m->m12 = -fs;	m->m13 = 0.0f;
	m->m21 = fs;	m->m22 =  fc;	m->m23 = 0.0f;
	m->m31 = 0.0f;  m->m32 = 0.0f;  m->m33 = 1.0f;
}

void ui_matrix_3x3_set_rotation_z_radians(ui_matrix_3x3_t m, float radians) {
	float fs = cpe_sin_radians(radians);
	float fc = cpe_cos_radians(radians);
    
	m->m11 = fc;	m->m12 = -fs;	m->m13 = 0.0f;
	m->m21 = fs;	m->m22 =  fc;	m->m23 = 0.0f;
	m->m31 = 0.0f;  m->m32 = 0.0f;  m->m33 = 1.0f;
}

ui_matrix_3x3 UI_MATRIX_3X3_IDENTITY = UI_INIT_IDENTITY_MATRIX_3X3;
ui_matrix_3x3 UI_MATRIX_3X3_ZERO = UI_INIT_ZERO_MATRIX_3X3;
