#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/stream.h"
#include "render/utils/ui_vector_4.h"

int ui_vector_4_cmp(ui_vector_4_t l, ui_vector_4_t r) {
    int rv;

    if ((rv = cpe_float_cmp(l->value[0], r->value[0], UI_FLOAT_PRECISION))) return rv;
    if ((rv = cpe_float_cmp(l->value[1], r->value[1], UI_FLOAT_PRECISION))) return rv;
    return cpe_float_cmp(l->value[2], r->value[2], UI_FLOAT_PRECISION);
}

void ui_vector_4_cross_product(ui_vector_4_t to, ui_vector_4_t l, ui_vector_4_t r) {
    to->x = l->x * r->x;
    to->y = l->y * r->y;
    to->z = l->z * r->z;
    to->w = l->w * r->w;
}

void ui_vector_4_inline_cross_product(ui_vector_4_t to, ui_vector_4_t o) {
    to->x *= o->x;
    to->y *= o->y;
    to->z *= o->z;
    to->w *= o->w;
}

void ui_vector_4_print(write_stream_t s, ui_vector_4_t v, const char * prefix) {
    stream_printf(s, "%s[ %4.2f, %4.2f, %4.2f, %4.2f ]", prefix, v->value[0], v->value[1], v->value[2], v->value[3]);
}

ui_vector_4 UI_VECTOR_4_ZERO = UI_VECTOR_4_INITLIZER( 0.0f,  0.0f,  0.0f, 0.0f);
ui_vector_4 UI_VECTOR_4_IDENTITY = UI_VECTOR_4_INITLIZER( 1.0f,  1.0f,  1.0f, 1.0f);
ui_vector_4 UI_VECTOR_4_NEGATIVE_ONE = UI_VECTOR_4_INITLIZER(-1.0f, -1.0f, -1.0f, -1.0f);
ui_vector_4 UI_VECTOR_4_POSITIVE_UNIT_X = UI_VECTOR_4_INITLIZER( 1.0f,  0.0f,  0.0f, 0.0f);
ui_vector_4 UI_VECTOR_4_POSITIVE_UNIT_Y = UI_VECTOR_4_INITLIZER( 0.0f,  1.0f,  0.0f, 0.0f);
ui_vector_4 UI_VECTOR_4_POSITIVE_UNIT_Z = UI_VECTOR_4_INITLIZER( 0.0f,  0.0f,  1.0f, 0.0f);
ui_vector_4 UI_VECTOR_4_POSITIVE_UNIT_W = UI_VECTOR_4_INITLIZER( 0.0f,  0.0f,  0.0f, 1.0f);
ui_vector_4 UI_VECTOR_4_NEGATIVE_UNIT_X = UI_VECTOR_4_INITLIZER(-1.0f,  0.0f,  0.0f, 0.0f);
ui_vector_4 UI_VECTOR_4_NEGATIVE_UNIT_Y = UI_VECTOR_4_INITLIZER( 0.0f, -1.0f,  0.0f, 0.0f);
ui_vector_4 UI_VECTOR_4_NEGATIVE_UNIT_Z = UI_VECTOR_4_INITLIZER( 0.0f,  0.0f, -1.0f, 0.0f);
ui_vector_4 UI_VECTOR_4_NEGATIVE_UNIT_W = UI_VECTOR_4_INITLIZER( 0.0f,  0.0f,  0.0f, -1.0f);
