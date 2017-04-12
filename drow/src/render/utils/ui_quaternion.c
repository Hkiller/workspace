#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/stream.h"
#include "render/utils/ui_quaternion.h"
#include "render/utils/ui_matrix_4x4.h"
#include "render/utils/ui_matrix_3x3.h"
#include "render/utils/ui_vector_3.h"
#include "render/utils/ui_vector_2.h"

int ui_quaternion_cmp(ui_quaternion_t l, ui_quaternion_t r) {
    int rv;

    if (l == r) return 0;
    
    if ((rv = cpe_float_cmp(l->w, r->w, UI_FLOAT_PRECISION))) return rv;
    if ((rv = cpe_float_cmp(l->x, r->x, UI_FLOAT_PRECISION))) return rv;
    if ((rv = cpe_float_cmp(l->y, r->y, UI_FLOAT_PRECISION))) return rv;
    return cpe_float_cmp(l->z, r->z, UI_FLOAT_PRECISION);
}

void ui_quaternion_to_rotation_matrix_4x4(ui_quaternion_t quaternion, ui_matrix_4x4_t m) {
	float fTx  = 2.0f * quaternion->x;
	float fTy  = 2.0f * quaternion->y;
	float fTz  = 2.0f * quaternion->z;
	float fTwx = fTx * quaternion->w;
	float fTwy = fTy * quaternion->w;
	float fTwz = fTz * quaternion->w;
	float fTxx = fTx * quaternion->x;
	float fTxy = fTy * quaternion->x;
	float fTxz = fTz * quaternion->x;
	float fTyy = fTy * quaternion->y;
	float fTyz = fTz * quaternion->y;
	float fTzz = fTz * quaternion->z;

	m->m11 = 1.0f-(fTyy+fTzz); m->m12 =       fTxy-fTwz;  m->m13 =       fTxz+fTwy;  m->m14 = 0.0f;
	m->m21 =       fTxy+fTwz;  m->m22 = 1.0f-(fTxx+fTzz); m->m23 =       fTyz-fTwx;  m->m24 = 0.0f;
	m->m31 =       fTxz-fTwy;  m->m32 =       fTyz+fTwx;  m->m33 = 1.0f-(fTxx+fTyy); m->m34 = 0.0f;
	m->m41 =            0.0f;  m->m42 =            0.0f;  m->m43 =            0.0f;  m->m44 = 1.0f;
}

void ui_quaternion_to_rotation_matrix_3x3(ui_quaternion_t quaternion, ui_matrix_3x3_t m) {
	float fTx  = 2.0f * quaternion->x;
	float fTy  = 2.0f * quaternion->y;
	float fTz  = 2.0f * quaternion->z;
	float fTwx = fTx * quaternion->w;
	float fTwy = fTy * quaternion->w;
	float fTwz = fTz * quaternion->w;
	float fTxx = fTx * quaternion->x;
	float fTxy = fTy * quaternion->x;
	float fTxz = fTz * quaternion->x;
	float fTyy = fTy * quaternion->y;
	float fTyz = fTz * quaternion->y;
	float fTzz = fTz * quaternion->z;

	m->m11 = 1.0f-(fTyy+fTzz); m->m12 =       fTxy-fTwz;  m->m13 =       fTxz+fTwy;
	m->m21 =       fTxy+fTwz;  m->m22 = 1.0f-(fTxx+fTzz); m->m23 =       fTyz-fTwx;
	m->m31 =       fTxz-fTwy;  m->m32 =       fTyz+fTwx;  m->m33 = 1.0f-(fTxx+fTyy);
}

void ui_quaternion_set_x_angle(ui_quaternion_t quaternion, float angle) {
    cpe_assert_float_sane(angle);
    
    angle *= 0.5f;
    
	quaternion->w = cpe_cos_angle(angle);
	quaternion->x = cpe_sin_angle(angle);
	quaternion->y = 0.0f;
	quaternion->z = 0.0f;
}

void ui_quaternion_set_x_radians(ui_quaternion_t quaternion, float radians) {
    cpe_assert_float_sane(radians);
    
    radians *= 0.5f;
    
	quaternion->w = cpe_cos_radians(radians);
	quaternion->x = cpe_sin_radians(radians);
	quaternion->y = 0.0f;
	quaternion->z = 0.0f;
}

void ui_quaternion_set_y_angle(ui_quaternion_t quaternion, float angle) {
    cpe_assert_float_sane(angle);
    
    angle *= 0.5f;
    
	quaternion->w = cpe_cos_angle(angle);
	quaternion->x = 0.0f;
	quaternion->y = cpe_sin_angle(angle);
	quaternion->z = 0.0f;
}

void ui_quaternion_set_y_radians(ui_quaternion_t quaternion, float radians) {
    cpe_assert_float_sane(radians);

    radians *= 0.5f;
    
	quaternion->w = cpe_cos_radians(radians);
	quaternion->x = 0.0f;
	quaternion->y = cpe_sin_radians(radians);
	quaternion->z = 0.0f;
}

void ui_quaternion_set_z_angle(ui_quaternion_t quaternion, float angle) {
    cpe_assert_float_sane(angle);
    
    angle *= 0.5f;
    
	quaternion->w = cpe_cos_angle(angle);
	quaternion->x = 0.0f;
	quaternion->y = 0.0f;
	quaternion->z = cpe_sin_angle(angle);
}

void ui_quaternion_set_z_radians(ui_quaternion_t quaternion, float radians) {
    cpe_assert_float_sane(radians);

    radians *= 0.5f;
    
	quaternion->w = cpe_cos_radians(radians);
	quaternion->x = 0.0f;
	quaternion->y = 0.0f;
	quaternion->z = cpe_sin_radians(radians);
}

void ui_quaternion_set_radians(ui_quaternion_t quaternion, ui_vector_3_t radians) {
    float x_sin, x_cos, y_sin, y_cos, z_sin, z_cos;
    float radians_x = radians->x * 0.5f;
    float radians_y = radians->y * 0.5f;
    float radians_z = radians->z * 0.5f;

    cpe_assert_float_sane(radians->x);
    cpe_assert_float_sane(radians->y);
    cpe_assert_float_sane(radians->z);

    x_sin = cpe_sin_radians(radians_x);
    x_cos = cpe_cos_radians(radians_x);

    y_sin = cpe_sin_radians(radians_y);
    y_cos = cpe_cos_radians(radians_y);

    z_sin = cpe_sin_radians(radians_z);
    z_cos = cpe_cos_radians(radians_z);
    
    quaternion->w = x_cos * y_cos * z_cos + x_sin * y_sin + z_sin;
    quaternion->x = x_sin * y_cos * z_cos - x_cos * y_sin + z_sin;    
    quaternion->y = x_cos * y_sin * z_cos + x_sin * y_cos + z_sin;    
    quaternion->y = x_cos * y_cos * z_sin - x_sin * y_sin + z_cos;
}

void ui_quaternion_get_radians(ui_quaternion_t q, ui_vector_3_t radians) {
    radians->x = atan2f(
        2.0f * (q->w * q->x + q->y * q->z),
        1.0f - 2.0f * (q->x * q->x + q->y * q->y)
        );
    radians->y = asinf(
        2.0f * (q->x * q->y - q->z * q->x));
    radians->z = atan2f(
        2.0f * (q->w * q->z + q->x * q->y),
        1.0f - 2.0f * (q->y * q->y + q->z * q->z));
}

void ui_quaternion_cross_product(ui_quaternion_t to, ui_quaternion_t l, ui_quaternion_t r) {
    to->w = l->w * r->w - l->x * r->x - l->y * r->y - l->z * r->z;
    to->x = l->w * r->x + l->x * r->w + l->y * r->z - l->z * r->y;
    to->y = l->w * r->y + l->y * r->w + l->z * r->x - l->x * r->z;
    to->z = l->w * r->z + l->z * r->w + l->x * r->y - l->y * r->x;
}

void ui_quaternion_inline_cross_product(ui_quaternion_t to, ui_quaternion_t o) {
	float qw = to->w, qx = to->x, qy = to->y, qz = to->z;
    
	to->w = qw * o->w - qx * o->x - qy * o->y - qz * o->z;
	to->x = qw * o->x + qx * o->w + qy * o->z - qz * o->y;
	to->y = qw * o->y + qy * o->w + qz * o->x - qx * o->z;
	to->z = qw * o->z + qz * o->w + qx * o->y - qy * o->x;
}

float ui_quaternion_length(ui_quaternion_t quaternion) {
    return sqrt(ui_quaternion_length_square(quaternion));
}

float ui_quaternion_length_square(ui_quaternion_t quaternion) {
    return quaternion->w * quaternion->w
        + quaternion->x * quaternion->x
        + quaternion->y * quaternion->y
        + quaternion->z * quaternion->z;
}

void ui_quaternion_get_normalize(ui_quaternion_t quaternion) {
    float length_square = ui_quaternion_length_square(quaternion);
    float inv_length;
    
    if (length_square <= 0.0f) {
        *quaternion = UI_QUATERNION_ZERO;
        return;
    }

    inv_length = 1.0f / sqrt(length_square);

    quaternion->w *= inv_length;
    quaternion->x *= inv_length;
    quaternion->y *= inv_length;
    quaternion->z *= inv_length;
}

void ui_quaternion_adj_vector_2(ui_quaternion_t q, ui_vector_2_t to, ui_vector_2_t i) {
    if (ui_quaternion_cmp(q, &UI_QUATERNION_IDENTITY) != 0) {
        ui_quaternion conjugate = UI_QUATERNION_INITLIZER(q->w, - q->x, - q->y, - q->z);
        ui_quaternion v = UI_QUATERNION_INITLIZER(0.0f, i->x, i->y, 0.0f);
        ui_quaternion r = *q;
        ui_quaternion_inline_cross_product(&r, &v);
        ui_quaternion_inline_cross_product(&r, &conjugate);

        to->x = r.x;
        to->y = r.y;
    }
    else {
        *to = *i;
    }
}

void ui_quaternion_inline_adj_vector_2(ui_quaternion_t q, ui_vector_2_t i) {
    if (ui_quaternion_cmp(q, &UI_QUATERNION_IDENTITY) != 0) {
        ui_quaternion conjugate = UI_QUATERNION_INITLIZER(q->w, - q->x, - q->y, - q->z);
        ui_quaternion v = UI_QUATERNION_INITLIZER(0.0f, i->x, i->y, 0.0f);
        ui_quaternion r = *q;
        ui_quaternion_inline_cross_product(&r, &v);
        ui_quaternion_inline_cross_product(&r, &conjugate);

        i->x = r.x;
        i->y = r.y;
    }
}

void ui_quaternion_adj_vector_3(ui_quaternion_t q, ui_vector_3_t to, ui_vector_3_t i) {
    if (ui_quaternion_cmp(q, &UI_QUATERNION_IDENTITY) != 0) {
        ui_quaternion conjugate = UI_QUATERNION_INITLIZER(q->w, - q->x, - q->y, - q->z);
        ui_quaternion v = UI_QUATERNION_INITLIZER(0.0f, i->x, i->y, i->z);
        ui_quaternion r = *q;
        ui_quaternion_inline_cross_product(&r, &v);
        ui_quaternion_inline_cross_product(&r, &conjugate);

        to->x = r.x;
        to->y = r.y;
        to->z = r.z;
    }
    else {
        *to = *i;
    }
}

void ui_quaternion_inline_adj_vector_3(ui_quaternion_t q, ui_vector_3_t i) {
    if (ui_quaternion_cmp(q, &UI_QUATERNION_IDENTITY) != 0) {
        ui_quaternion conjugate = UI_QUATERNION_INITLIZER(q->w, - q->x, - q->y, - q->z);
        ui_quaternion v = UI_QUATERNION_INITLIZER(0.0f, i->x, i->y, i->z);
        ui_quaternion r = *q;
        ui_quaternion_inline_cross_product(&r, &v);
        ui_quaternion_inline_cross_product(&r, &conjugate);

        i->x = r.x;
        i->y = r.y;
        i->z = r.z;
    }
}

void ui_quaternion_print(write_stream_t s, ui_quaternion_t t, const char * prefix) {
    stream_printf(s, "%s%3.2f(%3.2f, %3.2f, %3.2f)", prefix, t->w, t->x, t->y, t->z);    
}

ui_quaternion UI_QUATERNION_IDENTITY = UI_QUATERNION_INITLIZER(1.0f, 0.0f, 0.0f, 0.0f);
ui_quaternion UI_QUATERNION_ZERO = UI_QUATERNION_INITLIZER(0.0f, 0.0f, 0.0f, 0.0f);
