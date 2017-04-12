#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/stream.h"
#include "render/utils/ui_matrix_4x4.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_vector_3.h"

int ui_matrix_4x4_cmp(ui_matrix_4x4_t l, ui_matrix_4x4_t r) {
    uint8_t i;
    int rv;

    for(i = 0; i < CPE_ARRAY_SIZE(l->m); ++i) {
        if ((rv = cpe_float_cmp(l->m[i], r->m[i], UI_FLOAT_PRECISION))) return rv;
    }
    
    return 0;
}

void ui_matrix_4x4_inline_add(ui_matrix_4x4_t add_to, ui_matrix_4x4_t o) {
	add_to->m11 += o->m11; add_to->m12 += o->m12; add_to->m13 += o->m13; add_to->m14 += o->m14;
    add_to->m21 += o->m21; add_to->m22 += o->m22; add_to->m23 += o->m23; add_to->m24 += o->m24;
    add_to->m31 += o->m31; add_to->m32 += o->m32; add_to->m33 += o->m33; add_to->m34 += o->m34;
    add_to->m41 += o->m41; add_to->m42 += o->m42; add_to->m43 += o->m43; add_to->m44 += o->m44;
}

void ui_matrix_4x4_inline_sub(ui_matrix_4x4_t sub_to, ui_matrix_4x4_t o) {
	sub_to->m11 -= o->m11; sub_to->m12 -= o->m12; sub_to->m13 -= o->m13; sub_to->m14 -= o->m14;
    sub_to->m21 -= o->m21; sub_to->m22 -= o->m22; sub_to->m23 -= o->m23; sub_to->m24 -= o->m24;
    sub_to->m31 -= o->m31; sub_to->m32 -= o->m32; sub_to->m33 -= o->m33; sub_to->m34 -= o->m34;
    sub_to->m41 -= o->m41; sub_to->m42 -= o->m42; sub_to->m43 -= o->m43; sub_to->m44 -= o->m44;
}

void ui_matrix_4x4_inline_cross_product(ui_matrix_4x4_t to, ui_matrix_4x4_t o) {
    struct ui_matrix_4x4 r;
    ui_matrix_4x4_cross_product(&r, to, o);
    *to = r;
}

void ui_matrix_4x4_inline_mul_float(ui_matrix_4x4_t to, float f) {
    to->m11 *= f;  to->m12 *= f;  to->m13 *= f;  to->m14 *= f;
    to->m21 *= f;  to->m22 *= f;  to->m23 *= f;  to->m24 *= f;
    to->m31 *= f;  to->m32 *= f;  to->m33 *= f;  to->m34 *= f;
    to->m41 *= f;  to->m42 *= f;  to->m43 *= f;  to->m44 *= f;
}

void ui_matrix_4x4_inline_div_float(ui_matrix_4x4_t div_to, float f)  {
    div_to->m11 /= f;  div_to->m12 /= f;  div_to->m13 /= f;  div_to->m14 /= f;
    div_to->m21 /= f;  div_to->m22 /= f;  div_to->m23 /= f;  div_to->m24 /= f;
    div_to->m31 /= f;  div_to->m32 /= f;  div_to->m33 /= f;  div_to->m34 /= f;
    div_to->m41 /= f;  div_to->m42 /= f;  div_to->m43 /= f;  div_to->m44 /= f;
}

uint8_t ui_matrix_4x4_is_identity(ui_matrix_4x4_t m) {
    return (
        m->m11 == 1.0f && m->m12 == 0.0f && m->m13 == 0.0f && m->m14 == 0.0f &&
        m->m21 == 0.0f && m->m22 == 1.0f && m->m23 == 0.0f && m->m24 == 0.0f &&
        m->m31 == 0.0f && m->m32 == 0.0f && m->m33 == 1.0f && m->m34 == 0.0f &&
        m->m41 == 0.0f && m->m42 == 0.0f && m->m43 == 0.0f && m->m44 == 1.0f
        );
}

void ui_matrix_4x4_set_identity(ui_matrix_4x4_t m) {
	m->m11 = 1.0f; m->m12 = 0.0f; m->m13 = 0.0f; m->m14 = 0.0f;
	m->m21 = 0.0f; m->m22 = 1.0f; m->m23 = 0.0f; m->m24 = 0.0f;
	m->m31 = 0.0f; m->m32 = 0.0f; m->m33 = 1.0f; m->m34 = 0.0f;
	m->m41 = 0.0f; m->m42 = 0.0f; m->m43 = 0.0f; m->m44 = 1.0f;

}

void ui_matrix_4x4_cross_product(ui_matrix_4x4_t to, ui_matrix_4x4_t l, ui_matrix_4x4_t r) {
    to->m11 = l->m11 * r->m11 + l->m12 * r->m21 + l->m13 * r->m31 + l->m14 * r->m41;
    to->m12 = l->m11 * r->m12 + l->m12 * r->m22 + l->m13 * r->m32 + l->m14 * r->m42;
    to->m13 = l->m11 * r->m13 + l->m12 * r->m23 + l->m13 * r->m33 + l->m14 * r->m43;
    to->m14 = l->m11 * r->m14 + l->m12 * r->m24 + l->m13 * r->m34 + l->m14 * r->m44;
        
    to->m21 = l->m21 * r->m11 + l->m22 * r->m21 + l->m23 * r->m31 + l->m24 * r->m41;
    to->m22 = l->m21 * r->m12 + l->m22 * r->m22 + l->m23 * r->m32 + l->m24 * r->m42;
    to->m23 = l->m21 * r->m13 + l->m22 * r->m23 + l->m23 * r->m33 + l->m24 * r->m43;
    to->m24 = l->m21 * r->m14 + l->m22 * r->m24 + l->m23 * r->m34 + l->m24 * r->m44;
        
    to->m31 = l->m31 * r->m11 + l->m32 * r->m21 + l->m33 * r->m31 + l->m34 * r->m41;
    to->m32 = l->m31 * r->m12 + l->m32 * r->m22 + l->m33 * r->m32 + l->m34 * r->m42;
    to->m33 = l->m31 * r->m13 + l->m32 * r->m23 + l->m33 * r->m33 + l->m34 * r->m43;
    to->m34 = l->m31 * r->m14 + l->m32 * r->m24 + l->m33 * r->m34 + l->m34 * r->m44;
        
    to->m41 = l->m41 * r->m11 + l->m42 * r->m21 + l->m43 * r->m31 + l->m44 * r->m41;
    to->m42 = l->m41 * r->m12 + l->m42 * r->m22 + l->m43 * r->m32 + l->m44 * r->m42;
    to->m43 = l->m41 * r->m13 + l->m42 * r->m23 + l->m43 * r->m33 + l->m44 * r->m43;
    to->m44 = l->m41 * r->m14 + l->m42 * r->m24 + l->m43 * r->m34 + l->m44 * r->m44;
}

void ui_matrix_4x4_set_scale(ui_matrix_4x4_t m, float x, float y, float z) {
	m->m11 = x;    m->m12 = 0.0f, m->m13 = 0.0f; m->m14 = 0.0f;
	m->m21 = 0.0f; m->m22 = y;    m->m23 = 0.0f; m->m24 = 0.0f;
	m->m31 = 0.0f; m->m32 = 0.0f; m->m33 = z;    m->m34 = 0.0f;
	m->m41 = 0.0f; m->m42 = 0.0f; m->m43 = 0.0f; m->m44 = 1.0f;
}

void ui_matrix_4x4_set_scale_vector_3(ui_matrix_4x4_t m, ui_vector_3_t v) {
	m->m11 = v->x;  m->m12 = 0.0f, m->m13 = 0.0f; m->m14 = 0.0f;
	m->m21 = 0.0f; m->m22 = v->y;  m->m23 = 0.0f; m->m24 = 0.0f;
	m->m31 = 0.0f; m->m32 = 0.0f; m->m33 = v->z;  m->m34 = 0.0f;
	m->m41 = 0.0f; m->m42 = 0.0f; m->m43 = 0.0f; m->m44 = 1.0f;
}

void ui_matrix_4x4_set_scale_vector_2(ui_matrix_4x4_t m, ui_vector_2_t v) {
	m->m11 = v->x;  m->m12 = 0.0f, m->m12 = 0.0f; m->m14 = 0.0f;
	m->m21 = 0.0f; m->m22 = v->y;  m->m22 = 0.0f; m->m24 = 0.0f;
	m->m21 = 0.0f; m->m22 = 0.0f; m->m22 = 1.0f;  m->m24 = 0.0f;
	m->m41 = 0.0f; m->m42 = 0.0f; m->m42 = 0.0f; m->m44 = 1.0f;
}

void ui_matrix_4x4_set_translation(ui_matrix_4x4_t m, float x, float y, float z) {
    m->m11 = 1.0f; m->m12 = 0.0f; m->m13 = 0.0f; m->m14 = x;
    m->m21 = 0.0f; m->m22 = 1.0f; m->m23 = 0.0f; m->m24 = y;
    m->m31 = 0.0f; m->m32 = 0.0f; m->m33 = 1.0f; m->m34 = z;
    m->m41 = 0.0f; m->m42 = 0.0f; m->m43 = 0.0f; m->m44 = 1.0f;
}

void ui_matrix_4x4_set_translation_vector_3(ui_matrix_4x4_t m, ui_vector_3_t v) {
	m->m11 = 1.0f; m->m12 = 0.0f; m->m13 = 0.0f; m->m14 = v->x;
	m->m21 = 0.0f; m->m22 = 1.0f; m->m23 = 0.0f; m->m24 = v->y;
	m->m31 = 0.0f; m->m32 = 0.0f; m->m33 = 1.0f; m->m34 = v->z;
	m->m41 = 0.0f; m->m42 = 0.0f; m->m43 = 0.0f; m->m44 = 1.0f;
}

void ui_matrix_4x4_adj_vector_2(ui_matrix_4x4_t m, ui_vector_2_t to, ui_vector_2_t i) {
	to->x = m->m11 * i->x + m->m12 * i->y + m->m14;
    to->y = m->m21 * i->x + m->m22 * i->y + m->m24;
}

void ui_matrix_4x4_inline_adj_vector_2(ui_matrix_4x4_t m, ui_vector_2_t v) {
    struct ui_vector_2 to;
    
	to.x = m->m11 * v->x + m->m12 * v->y + m->m14;
    to.y = m->m21 * v->x + m->m22 * v->y + m->m24;

    cpe_assert_float_sane(to.x);
    cpe_assert_float_sane(to.y);

    *v = to;
}

void ui_matrix_4x4_adj_vector_3(ui_matrix_4x4_t m, ui_vector_3_t to, ui_vector_3_t v) {
    to->x = m->m11 * v->x + m->m12 * v->y + m->m13 * v->z + m->m14;
    to->y = m->m21 * v->x + m->m22 * v->y + m->m23 * v->z + m->m24;
    to->z = m->m31 * v->x + m->m32 * v->y + m->m33 * v->z + m->m34;
}

void ui_matrix_4x4_inline_adj_vector_3(ui_matrix_4x4_t m, ui_vector_3_t v) {
    struct ui_vector_3 to;
    
    to.x = m->m11 * v->x + m->m12 * v->y + m->m13 * v->z + m->m14;
    to.y = m->m21 * v->x + m->m22 * v->y + m->m23 * v->z + m->m24;
    to.z = m->m31 * v->x + m->m32 * v->y + m->m33 * v->z + m->m34;

    *v = to;
}

float ui_matrix_4x4_determinant(ui_matrix_4x4_t m) {
    return m->m[8] * m->m[5] * m->m[2] +
        m->m[4] * m->m[9] * m->m[2] +
        m->m[8] * m->m[1] * m->m[6] -
        m->m[0] * m->m[9] * m->m[6] -
        m->m[4] * m->m[1] * m->m[10] +
        m->m[0] * m->m[5] * m->m[10];
}

void ui_matrix_4x4_invers(ui_matrix_4x4_t to, ui_matrix_4x4_t origin) {
    float m00 = origin->m11, m01 = origin->m12, m02 = origin->m13, m03 = origin->m14;
    float m10 = origin->m21, m11 = origin->m22, m12 = origin->m23, m13 = origin->m24;
    float m20 = origin->m31, m21 = origin->m32, m22 = origin->m33, m23 = origin->m34;
    float m30 = origin->m41, m31 = origin->m42, m32 = origin->m43, m33 = origin->m44;

    float v0 = m20 * m31 - m21 * m30;
    float v1 = m20 * m32 - m22 * m30;
    float v2 = m20 * m33 - m23 * m30;
    float v3 = m21 * m32 - m22 * m31;
    float v4 = m21 * m33 - m23 * m31;
    float v5 = m22 * m33 - m23 * m32;

    float t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
    float t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
    float t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
    float t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

    float invDet = t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03;

    if (invDet == 0.0f) {
        assert(0);
        *to = *origin;
        return;
    }

    invDet = 1.0f / invDet;
    
    to->m11 = t00 * invDet;
    to->m21 = t10 * invDet;
    to->m31 = t20 * invDet;
    to->m41 = t30 * invDet;

    to->m12 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    to->m22 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    to->m32 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    to->m42 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    v0 = m10 * m31 - m11 * m30;
    v1 = m10 * m32 - m12 * m30;
    v2 = m10 * m33 - m13 * m30;
    v3 = m11 * m32 - m12 * m31;
    v4 = m11 * m33 - m13 * m31;
    v5 = m12 * m33 - m13 * m32;

    to->m13 = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    to->m23 = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    to->m33 = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    to->m43 = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    v0 = m21 * m10 - m20 * m11;
    v1 = m22 * m10 - m20 * m12;
    v2 = m23 * m10 - m20 * m13;
    v3 = m22 * m11 - m21 * m12;
    v4 = m23 * m11 - m21 * m13;
    v5 = m23 * m12 - m22 * m13;

    to->m14 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    to->m24 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    to->m34 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    to->m44 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;
}

void ui_matrix_4x4_inline_invers(ui_matrix_4x4_t to) {
    ui_matrix_4x4 tmp = *to;
    ui_matrix_4x4_invers(to, &tmp);
}

void ui_matrix_4x4_print(write_stream_t s, ui_matrix_4x4_t m, const char * prefix) {
    uint8_t i;
    for(i = 0; i < 4; i++) {
        stream_printf(
            s, "%s[ %3.2f, %3.2f, %3.2f, %3.2f ]\n",
            prefix, m->element[0][i], m->element[1][i], m->element[2][i], m->element[3][i]);
    }
}

ui_matrix_4x4 UI_MATRIX_4X4_IDENTITY = UI_INIT_IDENTITY_MATRIX_4X4;
ui_matrix_4x4 UI_MATRIX_4X4_ZERO = UI_INIT_ZERO_MATRIX_4X4;
