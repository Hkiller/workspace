#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_vector_2.h"

float ui_vector_2_length(ui_vector_2_t v) {
	return sqrt(v->x * v->x + v->y * v->y);
}

int ui_vector_2_cmp(ui_vector_2_t l, ui_vector_2_t r) {
    int rv;

    if ((rv = cpe_float_cmp(l->value[0], r->value[0], UI_FLOAT_PRECISION))) return rv;
    return cpe_float_cmp(l->value[1], r->value[1], UI_FLOAT_PRECISION);
}

void ui_vector_2_add(ui_vector_2_t to, ui_vector_2_t v1, ui_vector_2_t v2) {
    to->x = v1->x + v2->x;
    to->y = v1->y + v2->y;
}

void ui_vector_2_inline_add(ui_vector_2_t to_v, ui_vector_2_t o) {
    to_v->x += o->x;
    to_v->y += o->y;
}

void ui_vector_2_sub(ui_vector_2_t to, ui_vector_2_t v1, ui_vector_2_t v2) {
    to->x = v1->x - v2->x;
    to->y = v1->y - v2->y;
}

void ui_vector_2_inline_sub(ui_vector_2_t to_v, ui_vector_2_t o) {
    to_v->x -= o->x;
    to_v->y -= o->y;
}

void ui_vector_2_normalize(ui_vector_2_t to, ui_vector_2_t v) {
    float length = cpe_math_distance(0.0f, 0.0f, v->x, v->y);
    if (length) {
        float inv_length = 1.0f / length;
        to->x = v->x * inv_length;
        to->y = v->y * inv_length;
    }
    else {
        to->x = to->y = 0.0f;
    }
}

void ui_vector_2_inline_normalize(ui_vector_2_t v) {
    float length = cpe_math_distance(0.0f, 0.0f, v->x, v->y);
    if (length) {
        float inv_length = 1.0f / length;
        v->x *= inv_length;
        v->y *= inv_length;
    }
}

void ui_vector_2_cross_product(ui_vector_2_t to, ui_vector_2_t v1, ui_vector_2_t v2) {
	to->x = v1->x * v2->x;
    to->y = v1->y * v2->y;
}

void ui_vector_2_inline_cross_product(ui_vector_2_t to, ui_vector_2_t o) {
    to->x *= o->x;
    to->y *= o->y;
}

float ui_vector_2_dot_product(ui_vector_2_t v1, ui_vector_2_t v2) {
    return v1->x * v2->x + v1->y * v2->y;
}

ui_vector_2 UI_VECTOR_2_ZERO = UI_INIT_ZERO_VECTOR_2;
ui_vector_2 UI_VECTOR_2_IDENTITY = UI_INIT_IDENTITY_VECTOR_2;
ui_vector_2 UI_VECTOR_2_NEGATIVE_ONE = UI_VECTOR_2_INITLIZER(-1.0f, -1.0f);
ui_vector_2 UI_VECTOR_2_POSITIVE_UNIT_X = UI_VECTOR_2_INITLIZER( 1.0f,  0.0f);
ui_vector_2 UI_VECTOR_2_POSITIVE_UNIT_Y = UI_VECTOR_2_INITLIZER( 0.0f,  1.0f);
ui_vector_2 UI_VECTOR_2_NEGATIVE_UNIT_X = UI_VECTOR_2_INITLIZER(-1.0f,  0.0f);
ui_vector_2 UI_VECTOR_2_NEGATIVE_UNIT_Y = UI_VECTOR_2_INITLIZER( 0.0f, -1.0f);
